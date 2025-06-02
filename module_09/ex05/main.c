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
#define DP1         0b11101111
#define DP2         0b11011111
#define DP3         0b10111111
#define DP4         0b01111111

uint16_t d1 = 0;
uint16_t d2 = 0;
uint16_t d3 = 0;
uint16_t d4 = 0;

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

// ************************************************************* TIMER SETUP */
void timer1_init(void) {
    TCCR1B |= (1 << WGM12);                 // Configure timer 1 for CTC mode
    OCR1A = 15624;                          // Set CTC compare value (prescaler = 256)
    TCCR1B |= (1 << CS12) | (1 << CS10);    // Prescaler 1024
}

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
}

void display() {
    set_DP(segments[d1], DP1);
    clear_DP(DP1);
    set_DP(segments[d2], DP2);
    clear_DP(DP2);
    set_DP(segments[d3], DP3);
    clear_DP(DP3);
    set_DP(segments[d4], DP4);
    clear_DP(DP4);
}

void set_value() {
    static uint32_t i = 0;
    d1 = (i / 1000) % 10;
    d2 = (i / 100) % 10;
    d3 = (i / 10) % 10;
    d4 = i % 10;
    i++;
    if (i > 9999)
        i = 0;
}

int main() {
    i2c_init();
    timer1_init();
    set_value();
    while (1) {
        display();    
        if (TIFR1 & (1 << OCF1A)) {         // Check if OCR1A reached
            TIFR1 |= (1 << OCF1A); 
            set_value();                    // Clear OCF1A flag (write 1)
        }
    }
    return (0);
}
