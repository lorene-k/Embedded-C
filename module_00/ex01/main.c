#include <avr/io.h>

int main()
{
    DDRB |= (1 << PB0);     // Set bit 0 of DDRB to 1 = set PB0 as output
    PORTB |= (1 << PB0);    // Set bit 0 of PORTB to 1 = set PB0 to HIGH (5V = LED on)

    while (1) {             // Make sure LED stays on with infinite loop
    }
    return (0);
}