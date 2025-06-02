#include <avr/io.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest int
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

#define RED             "\e[1;31m"
#define GREEN           "\e[1;32m"
#define RESET           "\033[0m"
#define NEXT_LINE       "\033[1E"
#define CURSOR_LEFT     "\033[D"
#define MAGIC_EMPTY     0xFF
#define MAGIC_OCCUPIED  0x0C
#define MAGIC_END       0xEE
#define MAGIC_VAL       0xAA

#define BAD_INPUT   "Bad input - invalid format"
#define NO_SPACE    "No space left"
#define EXISTS      "Already exists"

char buf[100];
char cmd[100];
char key[100];
char value[100];
uint16_t address = 0;
uint8_t data = 0;
volatile uint8_t i = 0;
uint8_t bad_input = 0;

const char *cmds[5] = {
    "READ",
    "WRITE",
    "FORGET",
    "PRINT"
};

// ************************************************************** UART SETUP */
void uart_init() {
    UBRR0H = (unsigned char)(MYUBRR >> 8);  // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)MYUBRR;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);   // Enable receiver & transmitter
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // Set frame format to 8N1 (8-bit frame, 1 stop bit, no parity)
}

char uart_rx(void) {
    while (!(UCSR0A & (1 << RXC0)))         // Wait for data to be received
        ;
    return (UDR0);                          // Return received data from buffer
}

void uart_tx(const char c) {
    while (!(UCSR0A & (1 << UDRE0)))        // Wait for empty transmit buffer (if 0, buffer = full)
        ;
    UDR0 = c;                               // Put data into buffer, sends data
}

void uart_printstr(const char *str) {
    while (*str)
        uart_tx(*str++);
}

// ************************************************************ EEPROM SETUP */
unsigned char EEPROM_read(uint16_t address) {
    while (EECR & (1 << EEPE))  // Wait for completion of previous write
        ;
    EEAR = address;             // Set up address register
    EECR |= (1 << EERE);        // Start eeprom read by writing EERE
    return (EEDR);              // Return data from Data Register
}

void EEPROM_write(uint16_t address, unsigned char data) {
    while (EECR & (1 << EEPE))  // Wait for completion of previous write
        ;
    EEAR = address;             // Set up address register
    EEDR = data;                // Load data to register
    EECR |= (1 << EEMPE);       // Write logical 1 to eempe
    EECR |= (1 << EEPE);        // Start eeprom write by setting EEPE
}

// ********************************************************** DISPLAY EEPROM */
void print_hex_byte(unsigned char c) {
    const char hex_chars[] = "0123456789ABCDEF";
    uart_tx(hex_chars[(c >> 4) & 0x0F]);    // Extract upper and lower 4 bits + convert to hex
    uart_tx(hex_chars[c & 0x0F]);
}

void print_address(uint16_t i) {            // print 32-bit address = 8 hex digits (4 bits each)
    print_hex_byte(0);
    print_hex_byte(0);
    print_hex_byte(i >> 8);                 // display address (extract MSB)
    print_hex_byte(i & 0xFF);               // mask MSB to extract LSB
    uart_printstr("  ");
}

void print_bytes(uint16_t i) {
    for (uint16_t j = 0; j < 16; j++) {
        print_hex_byte(EEPROM_read(i + j));
        uart_tx(' ');
    }
}

void print_ascii(uint16_t i) {
    uart_printstr("  |");
    for (uint16_t j = 0; j < 16; j++) {
        unsigned char byte = EEPROM_read(i + j);
        uart_tx(' ');
        if (byte >= 32 && byte <= 126)
            uart_tx(byte);
        else
            uart_tx('.');
    }
    uart_tx('|');
}

void display_status() {                     // 1kbyte memory = 1024 bytes/addresses / address in range 0-255
    uart_printstr("\r\n\r\n");
    for (uint16_t i = 0; i < 1024; i += 16) {
        print_address(i);
        print_bytes(i);
        print_ascii(i);
        uart_printstr("\r\n");
    }
    uart_printstr("\r\n\r\n");
}

// *************************************************************** COMMANDS  */
void TEST(char *color, char *str) {
    uart_printstr(color);
    uart_printstr("\r\nTEST : ");
    uart_printstr(str);
    uart_printstr("\r\n");
    uart_printstr(RESET);
}

void print_response(char *color, char *str) {
    uart_printstr("\r\n");
    uart_printstr(color);
    uart_printstr(str);
    uart_printstr("\r\n");
    uart_printstr(RESET);
    bad_input = 0;
}

// **************************** READ */
uint8_t check_key(uint16_t *i) {
    char key_tmp[33];
    uint16_t j = 0;
    uint8_t byte = EEPROM_read(*i + j);
    while (byte != MAGIC_VAL) {
        key_tmp[j] = byte;
        j++;
        byte = EEPROM_read(*i + j);
    }
    key_tmp[j] = '\0';
    if (strcmp(key, key_tmp) == 0) {
        j++;
        byte = EEPROM_read(*i + j);
        uart_printstr(GREEN);
        uart_printstr("\r\n");
        while (byte != MAGIC_END) {
            uart_tx(byte);
            j++;
            byte = EEPROM_read(*i + j);
        }
        uart_printstr("\r\n");
        uart_printstr(RESET);
        return (1);
    }
    (*i) += j;
    return (0);
}

void handle_READ() {
    for (uint16_t i = 0; i < 1024; i++) {
        uint8_t magic = EEPROM_read(i);
        if (magic == MAGIC_OCCUPIED) {
            i++;
            if (check_key(&i))
                return ;
        }
    }
    print_response(GREEN, "empty");
}

// *************************** WRITE */
uint8_t check_space(uint16_t *i) {
    uint8_t space_needed = strlen(key) + strlen(value) + 3;
    uint8_t magic = 0;
    for (uint8_t j = 0; j < space_needed; j++) {
        magic = EEPROM_read(*i + j);
        if (magic != MAGIC_EMPTY) {
            *i += j;
            return (0);
        }
    } 
    return (1);
}

void write_pair(uint16_t i) {
    uint8_t address = i;
    EEPROM_write(i++, MAGIC_OCCUPIED);
    for (uint8_t j = 0; j < strlen(key); j++) {
        EEPROM_write(i++, key[j]);
    }
    EEPROM_write(i++, MAGIC_VAL);
    for (uint8_t j = 0; j < strlen(value); j++) {
        EEPROM_write(i++, value[j]);
    }
    EEPROM_write(i++, MAGIC_END);
    uart_printstr(GREEN);
    uart_printstr("\r\n");
    print_address(address);
    uart_printstr("\r\n\r\n");
    uart_printstr(RESET);
}

int16_t check_existing_pair() {
    uint8_t byte = 0;
    for (uint16_t i = 0; i < 1024; i++) {
        byte = EEPROM_read(i);
        if (byte == MAGIC_OCCUPIED) {
            char key_tmp[33];
            uint8_t j = 0;
            int16_t address = i;
            i++;
            byte = EEPROM_read(i);
            while (byte != MAGIC_VAL) {
                key_tmp[j] = byte;
                j++;
                i++;
                byte = EEPROM_read(i);
            }
            key_tmp[j] = '\0';
            if (strcmp(key, key_tmp) == 0)
                return (address);
        }
    }
    return (-1);
}

void handle_WRITE() {
    if (check_existing_pair() != -1) {
        print_response(GREEN, EXISTS);
        return ;
    }
    for (uint16_t i = 0; i < 1024; i++) {
        uint8_t magic = EEPROM_read(i);
        if (magic == MAGIC_EMPTY) {
            if (check_space(&i)) {
                write_pair(i);
                return ;
            }
        }
    }
    print_response(GREEN, NO_SPACE);
}

// ************************** FORGET */
void handle_FORGET() {
    int16_t address = check_existing_pair();
    if (address == -1)
        print_response(GREEN, "not found");
    else {
        uint8_t byte = EEPROM_read(address);
        uint8_t i = 0;
        while (byte != MAGIC_END) {
            EEPROM_write(address + i, 0xFF);
            i++;
            byte = EEPROM_read(address + i);
        }
        EEPROM_write(address + i, 0xFF);
    }
}

void handle_PRINT() {
    display_status();
}

void (*cmd_functions[])() = {
    handle_READ,
    handle_WRITE,
    handle_FORGET,
    handle_PRINT
};

void handle_cmd() {
    for (uint8_t i = 0; i < 4; i++) {
        if (strcmp(cmds[i], cmd) == 0) {
            cmd_functions[i]();
            return ;
        }
    }
}

// **************************************************************** PARSING  */
void check_arg_len() {
    if (strcmp(cmd, "PRINT") != 0
        && !(strlen(key) > 0 && strlen(key) <= 32))
        bad_input = 1;
    if (strcmp(cmd, "WRITE") == 0
        && !(strlen(value) > 0 && strlen(value) <= 32))
        bad_input = 1;
}

uint8_t quote_open = 0;
void extract_arg(char *dest, uint8_t *j) {
    while (buf[*j] && buf[*j] == ' ')
        (*j)++;
    while (buf[*j] && buf[*j] != '\"') 
        (*j)++;
    if (buf[*j] == '\"') {
        quote_open = 1;
        (*j)++;
    }
    while (buf[*j] && buf[*j] != '\"') {
        *dest = buf[*j];
        (*j)++;
        dest++;
    }
    *dest = '\0';
    if (buf[*j] == '\"') {
        quote_open = 0;
        (*j)++;
    }
}

void extract_cmd(char *dest, uint8_t *j) {
    while (buf[*j] && buf[*j] != ' ') {
        *dest = buf[*j];
        (*j)++;
        dest++;
    }
    *dest = '\0';
}

uint8_t tokenize() {
    uint8_t j = 0;
    if (i == 0)
        bad_input = 1;
    extract_cmd(cmd, &j);
    extract_arg(key, &j);
    extract_arg(value, &j);
    if (quote_open || cmd[0] == '\0')
        bad_input = 1;
    return (0);
}

void parse_input() {
    tokenize();
    if (bad_input == 1)
        return ;
    for (uint8_t i = 0; i < 4; i++) {
        if (strcmp(cmds[i], cmd) == 0) {
            check_arg_len();
            return ;
        }
    }
    bad_input = 1;
}

// ********************************************************* INPUT HANDLING  */
void handle_enter()
{
    buf[i] = '\0';
    parse_input();
    if (bad_input)
        print_response(RED, BAD_INPUT);
    else
        handle_cmd();
    uart_printstr("\r\nEEPROM> ");
    i = 0;
}

void handle_backspace() {
    if (i == 0)
        return ;
    uart_printstr(CURSOR_LEFT);
    uart_tx(' ');
    uart_printstr(CURSOR_LEFT);
    i--;
}

void handle_input() {
    char c = uart_rx();                     // Read received data
    if (c == 0x7F || c == 0x08)             // Handle backspace
        handle_backspace();
    else if (c == '\n' || c == '\r')        // Handle enter
        handle_enter();
    else if (i < 100) {                     // Collect input if in bounds
        buf[i] = c;
        uart_tx(c);
        i++;
    }
}

int main() {
    uart_init();
    display_status();
    uart_printstr("EEPROM> ");
    while (1) {
        handle_input();
    }
    return (0);
}
