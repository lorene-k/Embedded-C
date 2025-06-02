#include <avr/io.h>
#include <avr/eeprom.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest int
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

// ************************************************************** UART SETUP */
void uart_init() {
    UBRR0H = (unsigned char)(MYUBRR >> 8);  // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)MYUBRR;
    
    UCSR0B = (1 << TXEN0);                  // Enable transmitter
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // Set frame format to 8N1 (8-bit frame, 1 stop bit, no parity)
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

// **************************************************************** DISPLAY  */
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

void display_status() {                     // 1kbyte memory = 1024 bytes/addresses / address in range 0-255
    for (uint16_t i = 0; i < 1024; i += 16) {
        print_address(i);
        for (uint16_t j = 0; j < 16; j++) { // display 16 bytes at address
            print_hex_byte(EEPROM_read(i + j));
            uart_tx(' ');
        }
        uart_printstr("\r\n");
    }
}

int main() {
    uart_init();
    display_status();
    while (1)
        ;
    return (0);
}
