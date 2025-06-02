#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TARGET_TOP (F_CPU / 256) - 1
#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest integer
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);    // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)ubrr;
    
    UCSR0B = (1 << TXEN0);                  // Enable receiver
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // Set frame format to 8N1 (8-bit frame, 1 stop bit, no parity)
}

void timer_init() {
    TCCR1B |= (1 << WGM12);                 // Set CTC mode
    OCR1A = TARGET_TOP;                     // Set CTC compare value
    TIMSK1 |= (1 << OCIE1A);                // Enable Timer1 Compare Interrupt A in Timer Interrupt Mask Register
    TCCR1B |= (1 << CS12);                  // Start timer with prescaler = 256
    _delay_ms(100);
    sei();                                  // Enable global interrupts
}

void uart_tx(const char c) {
    while (!(UCSR0A & (1 << UDRE0)))        // Wait for empty transmit buffer (if 0, buffer = full)
        ;
    UDR0 = c;                               // Put data into buffer, sends data
}

ISR(TIMER1_COMPA_vect) {                    // Timer 1 Compare Interrupt Service Routine
    uart_tx('Z');                           // Send 'Z' over UART eat 1Hz frequency
}

int main() {
    uart_init(MYUBRR);
    timer_init();
    while (1)
        ;
    return (0);
}