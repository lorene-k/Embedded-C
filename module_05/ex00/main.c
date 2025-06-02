#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest integer
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

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

// *************************************************************** ADC SETUP */
void adc_init(void) {
    ADMUX = (1 << REFS0) | (1 << ADLAR);    // Set AVCC voltage reference, ADC Left Adjust Result
    ADCSRA = (1 << ADEN) | (1 << ADPS0)     // Enable ADC
        | (1 << ADPS1) | (1 << ADPS2);      // prescaler 128 (125kHz ADC clock frequency)
}

uint8_t adc_read(void) {
    ADMUX = (ADMUX & 0xF0) | 0x00;          // Select ADC0
    ADCSRA |= (1 << ADSC);                  // Start conversion
    while (ADCSRA & (1 << ADSC))            // Wait for conversion to complete
        ;
    return (ADCH);                          // Read 8-bit ADC result
}

// ********************************************************* CONVERT & PRINT */
void convert_and_print(uint8_t value) {
    const char hex_chars[] = "0123456789abcdef";

    // Extract upper and lower 4 bits + convert to hex
    uart_tx(hex_chars[(value >> 4) & 0x0F]);  
    uart_tx(hex_chars[value & 0x0F]);
    uart_tx('\n');
    uart_tx('\r');
}

int main() {
    uart_init();
    adc_init();
    sei();
    while (1) {
        uint8_t adc_value = adc_read();     // Read ADC value
        convert_and_print(adc_value);       // Convert and send ADC value
        _delay_ms(20);
    }
    return (0);
}
