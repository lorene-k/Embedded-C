#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest integer
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

char pot[3];
char ldr[3];
char ntc[3];

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
    ADMUX = (1 << REFS0) | (1 << ADLAR);    // Set AVCC voltage reference, ADC Left Adjust Result
    ADCSRA = (1 << ADEN) | (1 << ADPS0)     // Enable ADC
        | (1 << ADPS1) | (1 << ADPS2);      // prescaler 128 (125kHz ADC clock frequency)
}

uint8_t adc_read_pot(void) {
    ADMUX = (ADMUX & 0xF0) | 0x00;          // Select ADC0
    ADCSRA |= (1 << ADSC);                  // Start conversion
    while (ADCSRA & (1 << ADSC))            // Wait for conversion to complete
        ;
    return (ADCH);                          // Read 8-bit ADC result
}

uint8_t adc_read_ldr(void) {
    ADMUX = (ADMUX & 0xF0) | 0x01;          // Select ADC1
    ADCSRA |= (1 << ADSC);                  // Start conversion
    while (ADCSRA & (1 << ADSC))            // Wait for conversion to complete
        ;
    return (ADCH);                          // Read 8-bit ADC result
}

uint8_t adc_read_ntc(void) {
    ADMUX = (ADMUX & 0xF0) | 0x02;          // Select ADC2
    ADCSRA |= (1 << ADSC);                  // Start conversion
    while (ADCSRA & (1 << ADSC))            // Wait for conversion to complete
        ;
    return (ADCH);                          // Read 8-bit ADC result
}

// ********************************************************* CONVERT & PRINT */
void convert(uint8_t value, char *dest) {
        const char hex_chars[] = "0123456789abcdef";

        // Extract upper and lower 4 bits + convert to hex
        dest[0] = hex_chars[(value >> 4) & 0x0F];  
        dest[1] = hex_chars[value & 0x0F];
        dest[2] = '\0';
}

void print_result()
{
    uart_print_str(pot);
    uart_print_str(", ");
    uart_print_str(ldr);
    uart_print_str(", ");
    uart_print_str(ntc);
    uart_print_str("\n\r");
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
        uint8_t adc_pot_value = adc_read_pot();
        convert(adc_pot_value, pot);
        uint8_t adc_ldr_value = adc_read_ldr();
        convert(adc_ldr_value, ldr);
        uint8_t adc_ntc_value = adc_read_ntc();
        convert(adc_ntc_value, ntc);
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
