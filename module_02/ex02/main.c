#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest integer
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);    // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)ubrr;
    
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);   // Enable receiver & transmitter
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // Set frame format to 8N1 (8-bit frame, 1 stop bit, no parity)
}

void uart_tx(const char c) {
    while (!(UCSR0A & (1 << UDRE0)))        // Wait for empty transmit buffer (if 0, buffer = full)
        ;
    UDR0 = c;                               // Put data into buffer, sends data
}

char uart_rx(void) {
    while (!(UCSR0A & (1 << RXC0)))        // Wait for data to be received
        ;
    return (UDR0);                          // Return received data from buffer
}

int main() {
    uart_init(MYUBRR);
    while (1) {
        uart_tx(uart_rx());
    }
    return (0);
}