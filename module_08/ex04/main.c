#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

#define RED     "\e[1;31m"
#define GREEN   "\e[1;32m"
#define RESET   "\033[0m"
#define BAD_INPUT   "Bad input - invalid format"

#define DDR_SPI DDRB
#define SS      PB2             // Slave Select (SK9822 uses no SS, but keep low)
#define MOSI    PB3             // SPI MOSI (Data Out)
#define SCK     PB5             // SPI Clock
#define START   (uint8_t)0x00   // Start frame
#define END     (uint8_t)0xFF   // End frame

#define TOP_TIMER0 (F_CPU / 1024UL / 100)     // 10ms interrupt period = 156.25
#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

#define NEXT_LINE "\033[1E"
#define CURSOR_LEFT "\033[D"

volatile uint8_t i = 0;
volatile uint8_t bad_input = 0;
uint8_t rainbow = 0;
char input[13];
uint8_t color[3];
uint8_t led = 0;
uint8_t pos = 0;
static uint8_t prev_color6[3] = {0, 0, 0};
static uint8_t prev_color7[3] = {0, 0, 0};
static uint8_t prev_color8[3] = {0, 0, 0};

// ************************************************************** UART SETUP */
void uart_init() {
    UBRR0H = (unsigned char)(MYUBRR >> 8);    // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)MYUBRR;
    
    UCSR0B = (1 << RXEN0) | (1 << TXEN0)    // Enable receiver & transmitter
        | (1 << RXCIE0);             
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

void uart_printstr(const char *str) {
    while (*str)
        uart_tx(*str++);
}

// *************************************************************** SPI SETUP */
void SPI_master_init(void) {
    DDR_SPI = (1 << MOSI) | (1 << SCK)      // Set MOSI and SCK output, 
        | (1 << SS);                        // SS output - deactivate slave
    PORTB &= ~(1 << PB2);                   // Set SS output
    SPCR = (1 << SPE)| (1 << MSTR)          // Enable SPI & Master
        | (1 << SPR0);                      // Set clock rate f_osc/16
}

void SPI_master_transmit(char data) {
    SPDR = data;                            // Start transmission
    while(!(SPSR & (1 << SPIF)))            // Wait for transmission to complete
        ;
}

void set_transmit(uint8_t frame) {
    for (uint8_t i = 0; i < 4; i++)         // Send Start Frame (32 bits)
        SPI_master_transmit(frame);         // Then send LED frame
}

void set_color(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue) {
    SPI_master_transmit(0xE0 | (brightness & 0x1F));
    SPI_master_transmit(blue);
    SPI_master_transmit(green);
    SPI_master_transmit(red);
}

// ********************************************************** LED HANDLING */
void set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    set_color(10, r, g, b);
    set_color(10, r, g, b);
    set_color(10, r, g, b);
}

void wheel(uint8_t pos) {                   // Generates a smooth color transition
    pos = 255 - pos;                        // Inverts input value
    if (pos < 85) {                         // RED
        set_rgb(255 - pos * 3, 0, pos * 3); // Transitions from red to purple (mixing red and blue)
    } else if (pos < 170) {                 // GREEN 
        pos = pos - 85;
        set_rgb(0, pos * 3, 255 - pos * 3); // Transitions from purple to green (mixing blue and green)
    } else {
        pos = pos - 170;                    // BLUE
        set_rgb(pos * 3, 255 - pos * 3, 0); // Transitions from green to red (mixing red and green)
    }
}

void set_prev_color() {
    switch (led) {
        case 6:
            prev_color6[0] = color[0];
            prev_color6[1] = color[1];
            prev_color6[2] = color[2];
            break;
        case 7:
            prev_color7[0] = color[0];
            prev_color7[1] = color[1];
            prev_color7[2] = color[2];
            break;
        case 8:
            prev_color8[0] = color[0];
            prev_color8[1] = color[1];
            prev_color8[2] = color[2];
            break;
    }
}

void toggle_led(uint8_t led) {
    set_transmit(START);
    switch (led) {
        case 6:
            set_color(10, color[0], color[1], color[2]);
            set_color(10, prev_color7[0], prev_color7[1], prev_color7[2]);
            set_color(10, prev_color8[0], prev_color8[1], prev_color8[2]);
            break;
        case 7:
            set_color(10, prev_color6[0], prev_color6[1], prev_color6[2]);
            set_color(10, color[0], color[1], color[2]);
            set_color(10, prev_color8[0], prev_color8[1], prev_color8[2]);
            break;
        case 8:
            set_color(10, prev_color6[0], prev_color6[1], prev_color6[2]);
            set_color(10, prev_color7[0], prev_color7[1], prev_color7[2]);
            set_color(10, color[0], color[1], color[2]);
            break;
    }
    set_transmit(END);
}

void timer0_init() {
    TCCR0A |= (1 << WGM01);                 // CTC Mode
    TCCR0B |= (1 << CS02) | (1 << CS00);    // Prescaler 1024
    OCR0A = TOP_TIMER0;                     // Interrupt every ~10ms
    TIMSK0 |= (1 << OCIE0A);                // Enable Timer0 Compare Match A Interrupt
}

void set_mode() {
    if (!rainbow) {
        set_prev_color();
        toggle_led(led);
    }
}

ISR(TIMER0_COMPA_vect) {
    if (rainbow) {
        set_transmit(START);
        wheel(pos);
        set_transmit(END);
        pos++;
        if (pos == 255)
            pos = 0;
    }
}

// ***************************************************************** PARSING */
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

void extract_rgb()
{
    if (input[0] == '#' && i == 9) {        // Check valid #RRGGBB format & convert to int
        color[0] = char_to_int(input[1]) * 16 + char_to_int(input[2]);
        color[1] = char_to_int(input[3]) * 16 + char_to_int(input[4]);
        color[2] = char_to_int(input[5]) * 16 + char_to_int(input[6]);
    } else
        bad_input = 1;
}

void parse_input() {
    if (i == 0 || input[0] != '#') {
        bad_input = 1;
        return ;
    }
    if (strcmp(input, "#FULLRAINBOW") == 0)
        rainbow = 1;
    else {
        extract_rgb();
        if (input[7] == 'D') {
            led = atoi(&input[8]);
            if (led >= 6 && led <= 8)
            return ;
        }
        bad_input = 1;
    }
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
    uart_printstr(BAD_INPUT);
    uart_printstr(RESET);
    uart_printstr(NEXT_LINE);
    bad_input = 0;
}

void handle_enter()
{
    input[i] = '\0';
    uart_printstr(NEXT_LINE);
    parse_input();
    if (bad_input)
        handle_bad_input();
    else
        set_mode();
    i = 0;
}

ISR(USART_RX_vect) {  
    char c = UDR0;                      // Read received data
    rainbow = 0;
    if (c == 8 || c == 127)             // Handle backspace
        handle_backspace();
    else if (c == 10 || c == 13)        // Handle enter
        handle_enter();
    else if (i < 12)
    {
        uart_tx(c);
        input[i] = c;
        i++;
    }
}

int main() {
    SPI_master_init();
    uart_init();
    timer0_init();
    sei();
    while (1) {
    }
    return (0);
}