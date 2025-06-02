#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LED_R PD5
#define LED_G PD6
#define LED_B PD3

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

int main() {
    init_rgb();
    while (1) {
        for (int pos = 0; pos < 255; pos++) {
            wheel(pos);
            _delay_ms(20);
        }
    }
}