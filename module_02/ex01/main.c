#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TARGET_TOP (F_CPU / 1024) / 0.5 - 1
#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest integer
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

const char str[] = "Hello, World!\n\r";
volatile uint8_t i = 0;

void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);    // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)ubrr;
    
    UCSR0B = (1 << RXEN0) | (1 << TXEN0)    // Enable receiver and transmitter
        | (1 << TXCIE0);                    // Enable Transmit Complete Interrupt
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // Set frame format to 8N1 (8-bit frame, 1 stop bit, no parity)
}

void timer_init() {
    TCCR1B |= (1 << WGM12) | (1 << CS10)    // Set CTC mode & prescaler = 256
        | (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);                // Enable Timer1 Compare Interrupt A in Timer Interrupt Mask Register
    OCR1A = TARGET_TOP;                     // Set CTC compare value (prescaler = 256)
    sei();                                  // Enable global interrupts
}

void uart_printstr(const char *str) {
    i = 0;                                  // Reset index
    UDR0 = str[i];                          // Send character
}

ISR(TIMER1_COMPA_vect) {                    // Timer 1 Compare Interrupt Service Routine
    uart_printstr(str);                     // Send str every 2s
}

ISR(USART_TX_vect) {                        // TX Complete Interrupt
    i++;
    if (str[i] != '\0')
        UDR0 = str[i];                      // Send str character
}

int main()
{
    uart_init(MYUBRR);
    timer_init();
    while (1)
        ;
    return (0);
}