#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define RED     "\e[1;31m"
#define GREEN   "\e[1;32m"
#define RESET   "\033[0m"

#define LED_R PD5
#define LED_G PD6
#define LED_B PD3

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

#define NEXT_LINE "\033[1E"
#define CURSOR_LEFT "\033[D"

volatile uint8_t i = 0;
volatile uint8_t bad_input = 0;
char color[8];

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

// *************************************************************** RGB SETUP */
void init_rgb() {
    DDRD |= (1 << LED_R) | (1 << LED_G) | (1 << LED_B);

    // Timer 0 config - LED_R & LED_G
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1) // Set Compare Output Mode
        | (1 << WGM00) | (1 << WGM01);      // Fast PWM mode
    TCCR0B |= (1 << CS00);                  // No prescaler

    // Timer 2 config - LED_B
    TCCR2A |= (1 << COM2B1)                 // Set Compare Output Mode
        | (1 << WGM20) | (1 << WGM21);      // Fast PWM mode
    TCCR2B |= (1 << CS20);                  // No prescaler
}

void set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    OCR0B = r;
    OCR0A = g;
    OCR2B = b;
}

// ******************************************************** CONVERSION UTILS */
int char_to_int(char c)                     // Convert char to int in hex format, return -1 on error
{
    if (c >= '0' && c <= '9') {
        return (c - '0');
    } else if (c >= 'A' && c <= 'F') {
        return (c - 'A' + 10);
    } else if (c >= 'a' && c <= 'f') {
        return (c - 'a' + 10);
    }
    bad_input = 1;
    return (-1);
}

void extract_rgb(const char *color, int *r, int *g, int *b)
{
    if (color[0] == '#' && i == 7) {        // Check valid #RRGGBB format & convert to int
        *r = char_to_int(color[1]) * 16 + char_to_int(color[2]);
        *g = char_to_int(color[3]) * 16 + char_to_int(color[4]);
        *b = char_to_int(color[5]) * 16 + char_to_int(color[6]);
    } else
        bad_input = 1;
}

// ********************************************************** INPUT HANDLING */
void handle_backspace() {
    if (i == 0)
        return ;
    uart_printstr(CURSOR_LEFT);
    uart_tx(' ');
    uart_printstr(CURSOR_LEFT);
    i--;
}

void handle_bad_input() {
    uart_printstr(RED);
    uart_printstr("Invalid color format");
    uart_printstr(RESET);
    uart_printstr(NEXT_LINE);
    bad_input = 0;
}

void parse_input()
{
    color[i] = '\0';
    uart_printstr(NEXT_LINE);
    
    int r, g, b;
    extract_rgb(color, &r, &g, &b);
    if (bad_input)
        handle_bad_input();
    else
        set_rgb(r, g, b);
    i = 0;
}

ISR(USART_RX_vect) {  
    char c = UDR0;                      // Read received data
    if (c == 8 || c == 127)             // Handle backspace
        handle_backspace();
    else if (c == 10 || c == 13)        // Handle enter
        parse_input();
    else if (i < 7)
    {
        uart_tx(c);
        color[i] = c;
        i++;
    }
}

int main()
{
    init_rgb();
    uart_init(MYUBRR);
    sei();

    while (1)
        ;
    return (0);
}
