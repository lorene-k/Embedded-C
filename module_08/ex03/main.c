#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DDR_SPI DDRB
#define SS      PB2             // Slave Select (SK9822 uses no SS, but keep low)
#define MOSI    PB3             // SPI MOSI (Data Out)
#define SCK     PB5             // SPI Clock
#define START   (uint8_t)0x00   // Start frame
#define END     (uint8_t)0xFF   // End frame

const uint8_t colors[7][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 0, 255},
    {255, 255, 255}
};

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

void toggle_leds(uint8_t value) {
    if (value < 85) {
        set_color(0, 0, 0, 0);
        set_color(0, 0, 0, 0);
        set_color(0, 0, 0, 0);
    }
    else if (value < 170) {
        set_color(10, colors[6][0], colors[6][1], colors[6][2]);
        set_color(0, 0, 0, 0);
        set_color(0, 0, 0, 0);
    } else if (value < 255) {
        set_color(10, colors[6][0], colors[6][1], colors[6][2]);
        set_color(10, colors[6][0], colors[6][1], colors[6][2]);
        set_color(0, 0, 0, 0);
    } else if (value == 255) {
        set_color(10, colors[6][0], colors[6][1], colors[6][2]);
        set_color(10, colors[6][0], colors[6][1], colors[6][2]);
        set_color(10, colors[6][0], colors[6][1], colors[6][2]);
    }
}

int main() {
    SPI_master_init();
    adc_init();
    while (1) {
        uint8_t value = adc_read();
        set_transmit(START);
        toggle_leds(value);
        set_transmit(END);
    }
    return (0);
}