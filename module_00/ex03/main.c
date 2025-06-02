#include <avr/io.h>
#include <util/delay.h>

#define DEBOUNCE_DELAY 20

int main()
{
    DDRB |= (1 << PB0);     // Set bit 0 of DDRB to 1 = set PB0 as output (LED)
    DDRD &= ~(1 << DDD2);   // Set bit 2 of PORTD to 0 = set PD2 as input (button)
    PORTD |= (1 << PD2);    // Enable internal pull-up resistor to avoid floating pin
    
    uint8_t prev_state = (PIND & (1 << PD2)) != 0;      // Set initial button state (released)

    while (1) {
        uint8_t curr_state = (PIND & (1 << PD2)) != 0;  // Get current button state
        if (prev_state != curr_state) {                 // Check button state change
            _delay_ms(DEBOUNCE_DELAY);                  // Set delay to debounce button
            prev_state = curr_state;
            if (!prev_state) {                          // Check button state change
                PORTB ^= (1 << PB0);                    // Toggle bit 0 of PORTB (Reverse LED state)
            }
        }
    }
    return (0);
}