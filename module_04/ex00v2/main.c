#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SW1 PD2

volatile uint8_t button_pressed = 0;

void init_io() {
    DDRB |= (1 << PB0);                     // Set LED D5 as output
    DDRD &= ~((1 << DDD2));                 // Enable input for PD2
    PORTD |= (1 << SW1);                    // Enable pull-up resistor
}

void init_interrupts() {
    EICRA |= (1 << ISC01);                  // Any logical change on INT1 generates an interrupt request
    EIMSK |= (1 << INT0);                   // Enable INT0
    EIFR |= (1 << INTF0);                   // Clear INT0 flag
    sei();                                  // Enable global interrupts
}

ISR(INT0_vect) {                            // Interrupt for INT0 - button press
    PORTB ^= (1 << PB0);
    EIMSK &= ~(1 << INT0);                  // Disable INT0 to prevent multiple triggers
    _delay_ms(100);                         // Debounce delay
    EIFR |= (1 << INTF0);
    EIMSK |= (1 << INT0);                   // Enable INT0
}

int main()
{
    init_io();
    init_interrupts();
    while (1)
        ;
    return (0);
}
