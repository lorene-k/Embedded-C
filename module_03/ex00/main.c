#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LED_R PD5
#define LED_G PD6
#define LED_B PD3
#define MASK (PORTD & ~((1 << LED_R) | (1 << LED_G) | (1 << LED_B)))

const uint8_t colors[] = {
    (1 << LED_R),
    (1 << LED_G),
    (1 << LED_B),
};

int main()
{
    // Set PD5, PD6, PD3 as output
    DDRD |= (1 << LED_R) | (1 << LED_G) | (1 << LED_B);

    while (1) {
        for (int i = 0; i < 3; i++) {
            PORTD = MASK | colors[i];
            _delay_ms(1000);
        }
    }
}