#include <avr/io.h>

#define TOP_VALUE (F_CPU / 256) - 1
#define OFF_TIME (F_CPU / 256) * 0.9 - 1
#define ON_TIME (F_CPU / 256) * 0.1 - 1

int main()
{
    DDRB |= (1 << PB1);         // Set PB1 output
    
    // Set Output Compare A and B to toggle pins on match
    // Set PWM, Phase Correct mode + prescaler = 256
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS12);

    // Set TOP value for the timer = 1 cycle
    ICR1 = TOP_VALUE;

    // Set CTC compare values for off & on time
    OCR1A = ON_TIME;
    OCR1B = OFF_TIME;
    
    // Ensure program keeps running
    while (1) {
    }
}