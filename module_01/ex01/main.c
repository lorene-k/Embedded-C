#include <avr/io.h>

// Adjust frequency to 1Hz with prescaler & adjust timer to overflow after 0,5s
#define TARGET_TIMER_COUNT (F_CPU / 256) / 2 - 1

int main()
{
    DDRB |= (1 << PB1);         // Set PB1 output
    TCCR1B |= (1 << WGM12);     // Configure timer 1 for CTC mode
    TCCR1A |= (1 << COM1A0);    // Enable timer 1 Compare Output channel A in toggle mode
    OCR1A = TARGET_TIMER_COUNT; // Set CTC compare value (prescaler = 256)
    TCCR1B |= (1 << CS12);      // Start timer at (F_CPU / 256) - prescaler = 256

    while (1) {                 // Ensure program keeps running
    }
}