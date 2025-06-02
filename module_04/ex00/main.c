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
    EICRA |= (1 << ISC00);                  // Any logical change on INT1 generates an interrupt request
    EIMSK |= (1 << INT0);                   // Enable INT0
    EIFR |= (1 << INTF0);                   // Clear INT0 flag
    sei();                                  // Enable global interrupts
}

ISR(INT0_vect) {                            // Interrupt for INT0 - button press
    EIMSK &= ~(1 << INT0);                  // Disable INT0 to prevent multiple triggers
    TCCR0B |= (1 << CS02) | (1 << CS00);    // Set Timer0 prescaler to 1024 for debounce
    TCNT0 = 0;                              // Reset Timer0
    TIMSK0 |= (1 << TOIE0);                 // Enable Timer0 overflow interrupt
}

ISR(TIMER0_OVF_vect) {
    TIMSK0 &= ~(1 << TOIE0);                // Disable Timer0 interrupt
    TCCR0B &= ~((1 << CS02) | (1 << CS00)); // Stop Timer0

    button_pressed ^= 1;
    if (button_pressed == 1)
        PORTB ^= (1 << PB0);                // Toggle LED if button pressed

    EIFR |= (1 << INTF0);                   // Clear INT0 flag before re-enabling
    EIMSK |= (1 << INT0);                   // Re-enable INT0
}

int main()
{
    init_io();
    init_interrupts();
    while (1)
        ;
    return (0);
}
