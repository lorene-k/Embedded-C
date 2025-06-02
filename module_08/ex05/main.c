#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DEBOUNCE_DELAY 20
#define DDR_SPI DDRB
#define SS      PB2             // Slave Select (SK9822 uses no SS, but keep low)
#define MOSI    PB3             // SPI MOSI (Data Out)
#define SCK     PB5             // SPI Clock
#define START   (uint8_t)0x00   // Start frame
#define END     (uint8_t)0xFF   // End frame

uint8_t curr_color = 0;
uint8_t curr_led = 0;
uint8_t color[3];
static uint8_t prev_color6[3] = {0, 0, 0};
static uint8_t prev_color7[3] = {0, 0, 0};
static uint8_t prev_color8[3] = {0, 0, 0};

// *********************************************************** BUTTONS SETUP */
void buttons_init() {
    // Set PD2 & PD4 as inputs (for buttons)
    DDRD &= ~((1 << DDD2) | (1 << DDD4));
    // Enable internal pull-up resistors to avoid floating pins (for SW1 & SW2)
    PORTD |= (1 << PD2) | (1 << PD4);
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

// *************************************************************** SPI SETUP */
void SPI_master_init(void) {
    DDR_SPI = (1 << MOSI) | (1 << SCK)      // Set MOSI and SCK output, 
        | (1 << SS);                        // SS output - deactivate slave
    PORTB &= ~(1 << PB2);                   // Set SS output
    SPCR = (1 << SPE)| (1 << MSTR)          // Enable SPI & Master
        | (1 << SPR0);                      // Set clock rate f_osc/16
}

void SPI_master_transmit(char data) {
    SPDR = data;                            // Start transmission
    while(!(SPSR & (1 << SPIF)))            // Wait for transmission to complete
        ;
}

void set_transmit(uint8_t frame) {
    for (uint8_t i = 0; i < 4; i++)         // Send Start Frame (32 bits)
        SPI_master_transmit(frame);         // Then send LED frame
}

void set_color(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue) {
    SPI_master_transmit(0xE0 | (brightness & 0x1F));
    SPI_master_transmit(blue);
    SPI_master_transmit(green);
    SPI_master_transmit(red);
}

// ********************************************************** LED HANDLING */
void set_prev_color() {
    switch (curr_led) {
        case 0:
            prev_color6[0] = color[0];
            prev_color6[1] = color[1];
            prev_color6[2] = color[2];
            break;
        case 1:
            prev_color7[0] = color[0];
            prev_color7[1] = color[1];
            prev_color7[2] = color[2];
            break;
        case 2:
            prev_color8[0] = color[0];
            prev_color8[1] = color[1];
            prev_color8[2] = color[2];
            break;
    }
}

void toggle_led() {
    set_transmit(START);
    switch (curr_led) {
        case 0:
            set_color(10, color[0], color[1], color[2]);
            set_color(10, prev_color7[0], prev_color7[1], prev_color7[2]);
            set_color(10, prev_color8[0], prev_color8[1], prev_color8[2]);
            break;
        case 1:
            set_color(10, prev_color6[0], prev_color6[1], prev_color6[2]);
            set_color(10, color[0], color[1], color[2]);
            set_color(10, prev_color8[0], prev_color8[1], prev_color8[2]);
            break;
        case 2:
            set_color(10, prev_color6[0], prev_color6[1], prev_color6[2]);
            set_color(10, prev_color7[0], prev_color7[1], prev_color7[2]);
            set_color(10, color[0], color[1], color[2]);
            break;
    }
    set_transmit(END);
}

void update_leds(uint8_t value) {
    switch (curr_color) {
        case 0:
            color[0] = value;
            break;
        case 1:
            color[1] = value;
            break;
        case 2:
            color[2] = value;
            break;
    }
    set_prev_color();
    toggle_led(curr_led);
}

void check_buttons()
{
    // Change color if button SW1 (PD2) is pressed
    if (!(PIND & (1 << PD2))) {
        _delay_ms(DEBOUNCE_DELAY);
        if ((PIND & (1 << PD2)))
            curr_color = (curr_color + 1) % 3; 
    }
    // Change LED if button SW2 (PD3) is pressed
    else if (!(PIND & (1 << PD4))) {
        _delay_ms(DEBOUNCE_DELAY);
        if ((PIND & (1 << PD4)))
            curr_led = (curr_led + 1) % 3;
    }
}

int main() {
    SPI_master_init();
    buttons_init();
    adc_init();
    while (1) {
        uint8_t value = adc_read();
        check_buttons();
        update_leds(value);
    }
    return (0);
}