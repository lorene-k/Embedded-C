#include <avr/io.h>

int main()
{
    // Set PB1 as output
    DDRB |= (1 << PB1);

    while (1) {
        // Set PB1 to HIGH (LED on)
        PORTB ^= (1 << PB1);
        // loop for 64 iterations - 32 cycles per iteration
        for (volatile uint32_t i = 0; i < F_CPU / 64; i++) {
        }
    }
}
