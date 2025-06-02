#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DDR_SPI DDRB
#define SS      PB2  // Slave Select (SK9822 uses no SS, but keep low)
#define MOSI    PB3  // SPI MOSI (Data Out)
#define SCK     PB5  // SPI Clock
#define START   0x00 // Start frame
#define END     0xFF // End frame

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
        SPI_master_transmit(frame);          // Then send LED frame
}

void set_color(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue) {
    SPI_master_transmit(0xE0 | (brightness & 0x1F)); // 0 - 31
    SPI_master_transmit(blue);
    SPI_master_transmit(green);
    SPI_master_transmit(red);
}

int main() {
    SPI_master_init();
    set_transmit(START);
    set_color(20, 255, 0, 0);
    set_color(0, 0, 0, 0);
    set_color(0, 0, 0, 0);
    set_transmit(END);
    SPI_master_transmit(END);          // Send End Frame (32 bits - at least (LEDs/2) bytes)
    while (1)
        ;
    return (0);
}
