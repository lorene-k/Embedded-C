#include <avr/io.h>
#include <util/delay.h>

#define DEBOUNCE_DELAY 20
#define TOP (F_CPU / 256)

static void setup_io()
{
    // Set PB1 output (LED)
    DDRB |= (1 << PB1);

    // Set PD2 & PD4 as inputs (buttons)
    DDRD &= ~((1 << DDD2) | (1 << DDD4));

    // Enable internal pull-up resistors to avoid floating pins (for SW1 & SW2)
    PORTD |= (1 << PD2) | (1 << PD4);
}

static void setup_timer()
{
    // Set Output Compare A and B to toggle pins on match
    // Set PWM, Phase Correct mode + start prescaler = 256
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS12);

   // Set TOP value for the timer = 1 cycle
    ICR1 = TOP - 1;
}

static void update_duty_cycle(volatile uint8_t *value)
{
    // Adjust value if button SW1 (PD2) is pressed
    if (!(PIND & (1 << PD2))) {
        _delay_ms(DEBOUNCE_DELAY);
        if ((PIND & (1 << PD2)) && *value > 0){
            (*value) -= 10;
        }
    }
    // Adjust value if button SW2 (PD4) is pressed
    else if (!(PIND & (1 << PD4))) {
        _delay_ms(DEBOUNCE_DELAY);
        if ((PIND & (1 << PD4)) && *value < 90) {
            (*value) += 10;
        }
    }
}

int main()
{
    volatile uint8_t value = 90; // off time %

    setup_io();
    setup_timer();

    while (1) {
        OCR1A = ((TOP * (100 - value)) / 100) - 1;
        OCR1B = ((TOP * value) / 100) - 1;
        update_duty_cycle(&value);
    }
}