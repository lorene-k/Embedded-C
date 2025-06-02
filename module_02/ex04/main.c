#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TOP (F_CPU / 256) - 1

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

#define RED     "\e[1;31m"
#define GREEN   "\e[1;32m"
#define RESET   "\033[0m"

const char username[] = "cheese";
const char password[] = "bacon";
const char prompt[] = "Enter your login:\n\rusername: \n\rpassword: \n\r";

volatile uint8_t i = 0;
int typing_pw = 0;
int bad_input = 0;

// ************************************************************** UART SETUP */
void uart_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);    // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)ubrr;
    
    UCSR0B = (1 << RXEN0) | (1 << TXEN0)    // Enable receiver & transmitter
        | (1 << RXCIE0);                    // Enable RX Complete interrupt
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

// ********************************************************** INPUT HANDLING */
void handle_bad_input() {
    uart_printstr(RED);
    uart_printstr("Bad combination username/password\n\r");
    _delay_ms(2000);
    uart_printstr(RESET);
    uart_printstr("\033[0;0H\033[2J");
    uart_printstr(prompt);
    uart_printstr("\033[2;11H");
    bad_input = 0;
}

void handle_good_input() {
    while (1) {
        uart_printstr(GREEN);
        uart_printstr("SUCCESS!!!");
        PORTB ^= (1 << PB2);
        _delay_ms(50);
        PORTB ^= (1 << PB0);
        _delay_ms(50);
        PORTB ^= (1 << PB4);
        _delay_ms(50);
        PORTB ^= (1 << PB1);
        _delay_ms(50);
        PORTB ^= (1 << PB4);
        _delay_ms(50);
        uart_printstr("\033[4;0H\033[2K");
        PORTB ^= (1 << PB0);
        _delay_ms(50);
        PORTB ^= (1 << PB2);
        _delay_ms(50);
        PORTB ^= (1 << PB1);
        _delay_ms(50);
    }
}

void check_input(char c) {
    if (typing_pw && c != password[i])
        bad_input = 1;
    else if (!typing_pw && c != username[i])
        bad_input = 1;
}

void handle_backspace() {
    if (i == 0)
        return ;
    uart_printstr("\033[D");
    uart_tx(' ');
    uart_printstr("\033[D");
    i--;
}

void handle_enter()
{
    if (!typing_pw)
        uart_printstr("\033[3;11H");
    else
    {    
        uart_printstr("\033[4;1H");
        if (bad_input || i == 0)
        {    
            handle_bad_input();}
        else
            handle_good_input();
    }
    typing_pw ^= 1;
    i = 0;
}

ISR(USART_RX_vect) {  
    char c = UDR0;                          // Read received data
    if (c == 0x7F || c == 0x08)             // Handle backspace
        handle_backspace();
    else if (c == '\n' || c == '\r')        // Handle enter
        handle_enter();
    else if (i < 32) {
        if (typing_pw)
            uart_tx('*');
        else
            uart_tx(c);
        check_input(c);
        i++;
    }
}

int main() {
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);

    uart_init(MYUBRR);
    uart_printstr(prompt);
    uart_printstr("\033[2;11H");
    sei();

    while (1)
        ;
    return (0);
}
