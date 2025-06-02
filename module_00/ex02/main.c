#include <avr/io.h>
#include <util/delay.h>

int main()
{
    DDRB |= (1 << PB0);     // Set bit 0 of DDRB to 1 = set PB0 as output (LED)
    DDRD &= ~(1 << DDD2);   // Set bit 2 of PORTD to 0 = set PD2 as input (button)
    PORTD |= (1 << PD2);    // Activate internal pull-up resistor to avoid floating pin

    while (1) {
        if (!(PIND & (1 << PD2)))   // Check if bit 2 of PIND is set to 0 (button pressed)
            PORTB |= (1 << PB0);    // Set bit 0 of PORTB to 1 (LED on)
        else
            PORTB &= ~(1 << PB0);   // Set bit 0 of PORTB to 0 (LED off)
    }
    return (0);
}