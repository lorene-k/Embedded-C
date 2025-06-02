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
#define DP3_ON      0b10111111
#define DP4_ON      0b01111111

uint8_t segments[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

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

// ******************************************************7 SEGMENTS HANDLING */
void clear_DP(uint8_t dp) {
    write_data(OUTPUT_1, 0b00000000);       // Segments off
    write_data(CONF_1, 0b11111111);         // Set all segments as inputs
    write_data(OUTPUT_0, dp);               // Set DP3 off
    write_data(CONF_0, 0b11111111);         // Set DP3 as input
    _delay_ms(1);
}

void set_DP(uint8_t num, uint8_t dp) {
    write_data(CONF_0, dp);                 // Set DP3 as output
    write_data(OUTPUT_0, dp);               // Set DP3 as output (off)
    write_data(CONF_1, 0b10000000);         // Set all segments as outputs
    write_data(OUTPUT_1, num);              // Set number to display
    _delay_ms(1);
    clear_DP(dp);
}

int main() {
    i2c_init();
    uint8_t i = 0;
    while (1) {
        set_DP(segments[4], DP3_ON);
        set_DP(segments[2], DP4_ON);
    }
    return (0);
}
