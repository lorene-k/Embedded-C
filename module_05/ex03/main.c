#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest integer
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

char temp_str[5];

// ************************************************************** UART SETUP */
void uart_init() {
    UBRR0H = (unsigned char)(MYUBRR >> 8);  // Set baud rate in 16-bit USART Baud Rate Register
    UBRR0L = (unsigned char)MYUBRR;
    UCSR0B = (1 << TXEN0);                  // Enable transmitter
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01); // Set frame format to 8N1 (8-bit frame, 1 stop bit, no parity)
}

void uart_tx(const char c) {
    while (!(UCSR0A & (1 << UDRE0)))        // Wait for empty transmit buffer (if 0, buffer = full)
        ;
    UDR0 = c;                               // Put data into buffer, sends data
}

void uart_print_str(const char *str) {
    while (*str) {
        uart_tx(*str);
        str++;
    }
}

// *************************************************************** ADC SETUP */
void adc_init(void) {
    ADMUX = (1 << REFS0) | (1 << REFS1);    // Set Internal 1.1V Voltage Reference
    ADCSRA = (1 << ADEN) | (1 << ADPS0)     // Enable ADC
        | (1 << ADPS1) | (1 << ADPS2);      // prescaler 128 (125kHz ADC clock frequency)
}

uint16_t adc_read_temp(void) {
    ADMUX = (ADMUX & 0xF0) | 0x08;          // Select ADC0
    ADCSRA |= (1 << ADSC);                  // Start conversion
    while (ADCSRA & (1 << ADSC))            // Wait for conversion to complete
        ;
        return (ADCL | (ADCH << 8));        // Read 10-bit ADC result (ADCL + ADCH shifted left 8 bits)
}

// ********************************************************* CONVERT & PRINT */
void ft_itoa(uint16_t value, char *dest) {  // itoa conversion
    char tmp[5];
    uint8_t i = 0;
    if (value == 0) {
        dest[0] = '0';
        dest[1] = '\0';
        return;
    }
    while (value > 0) {
        tmp[i++] = value % 10 + '0';
        value /= 10;
    }
    tmp[i] = '\0';
    for (uint8_t j = 0; j < i; j++) {
        dest[j] = tmp[i - j - 1];
    }
    dest[i] = '\0';
}

void print_result()
{
    uart_print_str(temp_str);
    uart_print_str("\n\r");
}

// y = mx + b (y = tension, m = pente, x = température, b = ordonnée à l'origine)
// pour y = 347 (moyenne des valeurs de la tension)
// m = 1,0643 (moyenne taux de variation entre 2 points)
// x = 55 (moyenne des valeurs de la température en fonction de la tension)
// On trouve b = 287,4
// x1000 pour éviter les virgules
// temperature = (((value * 1000) - 287400 ) / 1064)
// Autre calcul : coef directeur * (valeur - point y de reference) + x de ref
uint16_t convert(uint16_t value) {
    return (((1064 * (value - 347)) / 1000) + 55);
}

// ******************************************************* TIMER & INTERRUPT */
void timer0_init(void) {
    TCCR0B = (1 << CS01) | (1 << CS00);     // Prescaler 64
    TIMSK0 |= (1 << TOIE0);                 // Enable Timer0 overflow interrupt
}

ISR(TIMER0_OVF_vect) {                      // Timer 0 Compare Interrupt
    static uint8_t ovf_count = 0;
    ovf_count++;
    if (ovf_count >= 20) {
        ovf_count = 0;
        uint16_t celsius_value = convert(adc_read_temp());
        ft_itoa(celsius_value, temp_str);
        print_result();
    }
}

int main() {
    uart_init();
    adc_init();
    timer0_init();
    sei();
    while (1)
        ;
    return (0);
}
