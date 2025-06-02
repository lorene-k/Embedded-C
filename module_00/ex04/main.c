#include <avr/io.h>
#include <util/delay.h>

#define DEBOUNCE_DELAY 80

static void setup_bits()
{
    // Set PB0, PB1, PB2 & PB4 outputs (for LEDs)
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);

    // Set PD2 & PD4 as inputs (for buttons)
    DDRD &= ~((1 << DDD2) | (1 << DDD4));

    // Enable internal pull-up resistors to avoid floating pins (for SW1 & SW2)
    PORTD |= (1 << PD2) | (1 << PD4);
}

static void update_value(volatile uint8_t *value)
{
    // Increment value if button SW1 (PD2) is pressed
    if (!(PIND & (1 << PD2))) {
        _delay_ms(DEBOUNCE_DELAY);
        if ((PIND & (1 << PD2)))
            (*value)++;
    }
    // Decrement value if button SW2 (PD3) is pressed
    else if (!(PIND & (1 << PD4))) {
        _delay_ms(DEBOUNCE_DELAY);
        if ((PIND & (1 << PD4)))
            (*value)--;
    }
    *value &= 0x0F;
}

static void display_value(volatile uint8_t value)
{
    // Combine bitmasks 00000111 + 00010000 & invert to clear PB0, PB1, PB2, PB4
    // Set first 3 bits according to least significant 3 bits of value
    // Set bit 4 according to bit 4 of value
    // PORTB = (PORTB & ~( (1 << 0) | (1 << 1) | (1 << 2) | (1 << 4) ))
    //    | (value & ((1 << 0) | (1 << 1) | (1 << 2)))
    //    | ((value & (1 << 3)) << 1);
    PORTB = (PORTB & ~(0x07 | (1 << PB4))) | (value & 0x07) | ((value & 0x08) << 1);
}

int main()
{
    volatile uint8_t value = 0;

    setup_bits();
    while (1) {
        display_value(value);
        update_value(&value);
    }
    return (0);
}