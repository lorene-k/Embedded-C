#include <avr/io.h>
#include <util/delay.h>

#define SLA_ADDR 0x20
#define SLA_W (SLA_ADDR << 1) | 0           // 0x70 = 8-bit address for write
#define SLA_R ((SLA_ADDR << 1) | 1)         // 0x71 = 8-bit address for read
#define OUTPUT_0 0x02
#define CONF_0 0x06

// ************************************************************** I2C SETUP */
void i2c_init(void) {
    TWBR = 72;                              // SCL frequency (100000) = F_CPU / (16 + 2(TWBR) x prescaler)
}

void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA)      // Send start condition
        | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))          // Wait for TWI Interrupt Flag set = START transmitted
        ;
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN)       // Transmit STOP condition
        | (1 << TWSTO);
}

void i2c_write(unsigned char data) {
    TWDR = data;                            // Send data = command to trigger measurement
    TWCR = (1 << TWINT) | (1 << TWEN);      // Clear TWINT bit in TWCR to start transmission of data
    while (!(TWCR & (1 << TWINT)))          // Wait for TWINT Flag set - indicates that SLA+W has been transmitted
        ;                                   // & ACK/NACK has been received
}

// ************************************************************ OUTPUT SETUP */
void write_data(uint8_t reg, uint8_t data) {
    i2c_start();
    i2c_write(SLA_W);
    i2c_write(reg);
    i2c_write(data);
    i2c_stop();
}

int main() {
    i2c_init();
    write_data(CONF_0, 0b11110111);         // Set O0.3 as output (led D9)
    uint8_t i = 0;
    while (1) {
        i++;
        if (i % 2 == 0)                     // Toggle led
            write_data(OUTPUT_0, 0b11111111);
        else
            write_data(OUTPUT_0, 0b11110111);
        _delay_ms(500);
    }
    return (0);
}
