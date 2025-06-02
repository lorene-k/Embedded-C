#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LED_R PD5
#define LED_G PD6
#define LED_B PD3
#define MASK (PORTB & ~(0x07 | (1 << PB4)))

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest integer
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

// *************************************************************** ADC SETUP */
void adc_init(void) {
    ADMUX = (1 << REFS0) | (1 << ADLAR);    // Set AVCC voltage reference, ADC Left Adjust Result
    ADCSRA = (1 << ADEN) | (1 << ADPS0)     // Enable ADC
        | (1 << ADPS1) | (1 << ADPS2);      // prescaler 128 (125kHz ADC clock frequency)
}

uint8_t adc_read(void) {
    ADMUX = (ADMUX & 0xF0) | 0x00;          // Select ADC0
    ADCSRA |= (1 << ADSC);                  // Start conversion
    while (ADCSRA & (1 << ADSC))            // Wait for conversion to complete
        ;
    return (ADCH);                          // Read 8-bit ADC result
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

// ******************************************************************** LEDS */
void display(uint8_t value)
{
    if (value < 64)
        PORTB = MASK;
    else if (value > 63 && value < 128)
        PORTB = MASK | 0x01;
    else if (value > 127 && value < 192)
        PORTB = MASK | 0x03;
    else if (value > 191 && value < 255)
        PORTB = MASK | 0x07;
    else if (value == 255)
        PORTB = 0x07 | (1 << PB4);
}

int main() {
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
    init_rgb();
    adc_init();
    while (1) {
        uint8_t value = adc_read();
        wheel(value);
        display(value);
    }
    return (0);
}
