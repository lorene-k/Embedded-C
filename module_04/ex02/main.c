#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DEBOUNCE_DELAY 80
#define MASK (PORTB & ~(0x07 | (1 << PB4)))

volatile uint8_t value = 0;

// ******************************************************************* SETUP */
void setup_io() {
    // Set PB0, PB1, PB2 & PB4 as outputs
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
    DDRD &= ~((1 << DDD2) | (1 << DDD4));   // Set PD2 & PD4 as inputs
    PORTD |= (1 << PD2) | (1 << PD4);       // Enable internal pull-up resistors for SW1 & SW2
}

void init_interrupts() {
    //SW1 setup
    EICRA |= (1 << ISC01);                  // Interrupt on logical change for INT0
    EIMSK |= (1 << INT0);                   // Enable INT0
    EIFR |= (1 << INTF0);                   // Clear INT0 flag
    // SW2 setup
    PCICR |= (1 << PCIE2);                  // Enable Pin Change Interrupt for PCINT20
    PCMSK2 |= (1 << PCINT20);               // Enable PCINT20
    PCIFR |= (1 << PCIF2);                  // Clear PCINT20 flag
    sei();                                  // Enable global interrupts
}

static void display_value(volatile uint8_t value)
{
    // Combine bitmasks 00000111 + 00010000 & invert to clear PB0, PB1, PB2, PB4
    // Set first 3 bits according to least significant 3 bits of value
    // Set bit 4 according to bit 4 of value
    PORTB = MASK | (value & 0x07) | ((value & 0x08) << 1);
}

// ********************************************************************* SW1 */
ISR(INT0_vect) {                            // Interrupt for INT0 - button press
    EIMSK &= ~(1 << INT0);                  // Disable INT0 to prevent multiple triggers
    TCCR0B |= (1 << CS02) | (1 << CS00);    // Set Timer0 prescaler to 1024 for debounce
    TCNT0 = 0;                              // Reset Timer0
    TIMSK0 |= (1 << TOIE0);                 // Enable Timer0 overflow interrupt
}

ISR(TIMER0_OVF_vect) {
    TIMSK0 &= ~(1 << TOIE0);                // Disable Timer0 interrupt
    TCCR0B &= ~((1 << CS02) | (1 << CS00)); // Stop Timer0

    if (!((PIND & (1 << PD2))) && value < 15) {
        value++;                            // Increment value
        display_value(value);               // Update display
    }
    EIFR |= (1 << INTF0);                   // Clear INT0 flag before re-enabling
    EIMSK |= (1 << INT0);                   // Re-enable INT0
}

// ********************************************************************* SW2 */
ISR(PCINT2_vect) {                          // Interrupt for SW2
    PCMSK2 &= ~(1 << PCINT20);              // Disable PCINT20
    TCCR2B |= (1 << CS22) | (1 << CS20);    // Set Timer2 prescaler to 1024 for debounce
    TCNT2 = 0;                              // Reset Timer2
    TIMSK2 |= (1 << TOIE2);                 // Enable Timer2 overflow interrupt
}

ISR(TIMER2_OVF_vect) {
    TIMSK2 &= ~(1 << TOIE2);                // Disable Timer2 interrupt
    TCCR2B &= ~((1 << CS22) | (1 << CS20)); // Stop Timer2

    if (!((PIND & (1 << PD4))) && value > 0) {
        value--;                            // Decrement value
        display_value(value);               // Update display
    }
    PCIFR |= (1 << PCIF2);                  // Clear PCINT20 flag
    PCMSK2 |= (1 << PCINT20);               // Enable PCINT20
}

int main()
{
    setup_io();
    init_interrupts();
    while (1)
        ;
    return (0);
}
