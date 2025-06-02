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
#define MAGIC_DELETED   0xDE
#define MAGIC_EMPTY     0xFF
#define MAGIC_OCCUPIED  0x0C
#define MAGIC_END       0xEE
#define MAGIC_VAL       0xAA

#define BAD_INPUT   "Bad input - invalid format"
#define NO_SPACE    "\r\nNo space left"
#define EXISTS      "\r\nAlready exists"

char buf[100];
char cmd[100];
char key[100];
char value[100];
uint16_t address = 0;
uint8_t data = 0;
volatile uint8_t i = 0;
uint8_t bad_input = 0;


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
    unsigned char byte = 0;
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
void clear_EEPROM() {
    uint8_t byte = 0;
    for (uint16_t i = 0; i < 1024; i++) {
        byte = EEPROM_read(i);
        if (byte != 0xFF)
            EEPROM_write(i, 0xFF);
    }
}

int main() {
    uart_init();
    uart_printstr(GREEN);
    display_status();
    uart_printstr(RESET);
    clear_EEPROM();
    display_status();
    while (1) {
    }
    return (0);
}