#include <avr/io.h>
#include <util/delay.h>

#define SLA_ADDR    0x20
#define SLA_R       ((SLA_ADDR << 1) | 1)   // 0x71 = 8-bit address for read
#define SLA_W       (SLA_ADDR << 1) | 0     // 0x70 = 8-bit address for write
#define CONF_0      0x06
#define INPUT_0     0x00
#define OUTPUT_0    0x02
#define CONF_1      0x07
#define INPUT_1     0x01
#define OUTPUT_1    0x03

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

unsigned char i2c_read(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);  // Enable ACK (set TWEA)
    while (!(TWCR & (1 << TWINT)))
        ;
    return (TWDR);
}

// ************************************************************ I/O HANDLING */
void write_data(uint8_t reg, uint8_t data) {
    i2c_start();
    i2c_write(SLA_W);
    i2c_write(reg);
    i2c_write(data);
    i2c_stop();
}

unsigned char read_data(uint8_t reg) {
    i2c_start();
    i2c_write(SLA_W);
    i2c_write(reg);
    i2c_start();
    i2c_write(SLA_R);
    unsigned char data = i2c_read();
    i2c_stop();
    return (data);
}

int main() {
    i2c_init();
    write_data(CONF_0, 0b01111111);         // Set DP4 as output
    write_data(OUTPUT_0, 0b01111111);       // Set DP4 as output
    write_data(CONF_1, 0b10100100);         // Set segments a, b, g, e, d as outputs
    write_data(OUTPUT_1, ~(0b10100100));       // Set DP4 as output
    while (1)
        ;
    return (0);
}
