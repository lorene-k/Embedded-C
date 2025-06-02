#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define TOP_TIMER0 (F_CPU / 1024UL / 100)     // 10ms interrupt period = 156.25
#define TOP_TIMER1 (F_CPU / (256UL * 500))    // 500Hz PWM frequency (500 overflows/s)

// ************************************************************* TIMER SETUP */
void setup_timer1() {
   DDRB |= (1 << PB1);                      // Set PB1 output (LED)
   TCCR1A |= (1 << COM1A1) | (1 << WGM11);  // Set Fast PWM - prescaler 256
   TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS12);
   ICR1 = TOP_TIMER1;                       // Set TOP value
   OCR1A = 0;                               // Start with 0% duty cycle
}

void setup_timer0() {
    TCCR0A |= (1 << WGM01);                 // CTC Mode
    TCCR0B |= (1 << CS02) | (1 << CS00);    // Prescaler 1024
    OCR0A = TOP_TIMER0;                     // Interrupt every ~10ms
    TIMSK0 |= (1 << OCIE0A);                // Enable Timer0 Compare Match A Interrupt
}

// ************************************************************** INTERRUPTS */
ISR(TIMER0_COMPA_vect) {                    // Interrupt for Timer0 : update duty cycle 1%
    static int8_t direction = 1;            // Increasing or decreasing duty cycle
    static uint8_t duty_cycle = 0;
    
    duty_cycle += direction * 2;            // Increment or decrement by 2%
    OCR1A = (ICR1 * duty_cycle) / 100;      // Convert % to actual PWM value
    
    if (duty_cycle == 100 || duty_cycle == 0)
        direction = -direction;             // Reverse direction at 0% or 100%
}

int main()
{
    setup_timer1();
    setup_timer0();
    sei();
    while (1)
        ;
    return (0);
}

/*
Timer frequency = F_CPU / prescaler
1 tick = 1/15625s = 64us
10ms = 10000us / 64us = 156.25 ticks

Overflow time = 256 / timer frequency = 16.384ms
Overflows per second = 1 / overflow time = 61.035 overflows

Timer frequencies (= ticks/s)
F_CPU / 1024 = 15625Hz
F_CPU / 256 = 62500Hz
F_CPU / 64 = 250000Hz
F_CPU / 8 = 2000000Hz

Overflow time for 8-bit timer:
1024 prescaler = 16.384ms
256 prescaler = 4.096ms
64 prescaler = 1.024ms
8 prescaler = 0.128ms

Overflow time for 16-bit timer:
1024 prescaler = 65.536ms
256 prescaler = 16.384ms
64 prescaler = 4.096ms
8 prescaler = 0.512ms

Overflows per second for 8-bit timer:
1024 prescaler = 61.035 overflows
256 prescaler = 244.141 overflows
64 prescaler = 976.562 overflows
8 prescaler = 7812.5 overflows

Overflows per second for 16-bit timer:
1024 prescaler = 15.259 overflows
256 prescaler = 61.035 overflows
64 prescaler = 244.141 overflows
8 prescaler = 1953.125 overflows
*/