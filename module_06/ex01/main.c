#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest int
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

#define SLA_ADDR 0x38
#define SLA_W (SLA_ADDR << 1) | 0           // 0x70 = 8-bit address for write
#define SLA_R ((SLA_ADDR << 1) | 1)         // 0x71 = 8-bit address for read

uint8_t data[7];

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

void uart_printstr(const char *str) {
    while (*str)
        uart_tx(*str++);
}

// ************************************************************ STATUS CODES */
void print_status(uint8_t status_code)
{
    if (status_code == TW_START)
        uart_printstr("START acknowledge.");
    else if (status_code == TW_REP_START)
        uart_printstr("REPEATED START acknowledge.");
    else if (status_code == TW_MT_SLA_ACK)
        uart_printstr("Master Transmitter : Slave ACK");
    else if (status_code == TW_MT_SLA_NACK)
        uart_printstr("Master Transmitter : Slave NACK");
    else if (status_code == TW_MT_DATA_ACK)
        uart_printstr("Master Transmitter : Data ACK");
    else if (status_code == TW_MT_DATA_NACK)
        uart_printstr("Master Transmitter : Data NACK");
    else if (status_code == TW_MR_SLA_ACK)
        uart_printstr("Master Receiver : Slave ACK");
    else if (status_code == TW_MR_SLA_NACK)
        uart_printstr("Master Receiver : Slave NACK");
    else if (status_code == TW_MR_DATA_ACK)
        uart_printstr("Master Receiver : Data ACK");
    else if (status_code == TW_MR_DATA_NACK)
        uart_printstr("Master Receiver : Data NACK");
    else if (status_code == TW_MT_ARB_LOST || status_code == TW_MR_ARB_LOST)
        uart_printstr("Arbitration Lost");
    else if (status_code == TW_ST_SLA_ACK)
        uart_printstr("Slave Transmitter : Slave ACK");
    else if (status_code == TW_ST_ARB_LOST_SLA_ACK)
        uart_printstr("Arbitration Lost in SLA+R/W, Slave ACK");
    else if (status_code == TW_ST_DATA_ACK)
        uart_printstr("Slave Transmitter : Data ACK");
    else if (status_code == TW_ST_DATA_NACK)
        uart_printstr("Slave Transmitter : Data NACK");
    else if (status_code == TW_ST_LAST_DATA)
        uart_printstr("Slave Transmitter : Last Data");
    else if (status_code == TW_SR_SLA_ACK)
        uart_printstr("Slave Receiver : Slave ACK");
    else if (status_code == TW_SR_ARB_LOST_SLA_ACK)
        uart_printstr("Arbitration Lost in SLA+R/W, Slave ACK");
    else if (status_code == TW_SR_GCALL_ACK)
        uart_printstr("General Call : Slave ACK");
    else if (status_code == TW_SR_ARB_LOST_GCALL_ACK)
        uart_printstr("Arbitration Lost in General Call, Slave ACK");
    else if (status_code == TW_SR_DATA_ACK)
        uart_printstr("Slave Receiver : Data ACK");
    else if (status_code == TW_SR_DATA_NACK)
        uart_printstr("Slave Receiver : Data NACK");
    else if (status_code == TW_SR_GCALL_DATA_ACK)
        uart_printstr("General Call : Data ACK");
    else if (status_code == TW_SR_GCALL_DATA_NACK)
        uart_printstr("General Call : Data NACK");
    else if (status_code == TW_SR_STOP)
        uart_printstr("Slave Receiver : STOP received");
    else if (status_code == TW_NO_INFO)
        uart_printstr("No state information available");
    else if (status_code == TW_BUS_ERROR)
        uart_printstr("Bus Error");
    else
        uart_printstr("Unknown Status Code");
    uart_printstr("\r\n");
}

// *************************************************************** I2C SETUP */
void i2c_write(unsigned char data) ;
void i2c_read(void) ;

void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA)      // Send start condition
        | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))          // Wait for TWI Interrupt Flag set = START transmitted
    ;
}

void i2c_calibrate(void) {
    i2c_write(SLA_W);
    i2c_write(0x71);
    i2c_write(SLA_R);
    i2c_read();
    uint8_t status_word = TWDR;
    if ((status_word & 0x08) == 0) {
        i2c_write(SLA_W);
        i2c_write(0xBE);
        i2c_write(0x08);
        i2c_write(0x00);
        _delay_ms(10);
    }
}

void i2c_init(void) {
    TWBR = 72;                              // SCL frequency (100000) = F_CPU / (16 + 2(TWBR) x prescaler)
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN)       // Transmit STOP condition
        | (1 << TWSTO);
}

// ************************************************ I2C WRITE - READ - PRINT */
void i2c_write(unsigned char data) {
    TWDR = data;                            // Send data = command to trigger measurement
    TWCR = (1 << TWINT) | (1 << TWEN);      // Clear TWINT bit in TWCR to start transmission of data
    while (!(TWCR & (1 << TWINT)))          // Wait for TWINT Flag set - indicates that SLA+W has been transmitted
        ;                                   // & ACK/NACK has been received
}

void i2c_read(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);  // Enable ACK (set TWEA)
    while (!(TWCR & (1 << TWINT)))
        ;
}

void print_hex_value(char c) {
    const char hex_chars[] = "0123456789ABCDEF";
    uart_tx(hex_chars[(c >> 4) & 0x0F]);    // Extract upper and lower 4 bits + convert to hex
    uart_tx(hex_chars[c & 0x0F]);
    uart_tx(' ');
}

int main() {
    uart_init();
    i2c_init();
    i2c_start();
    i2c_calibrate();
    while (1) {
        i2c_start();
        i2c_write(SLA_W); 
        i2c_write(0xAC);                    // "Send the 0xAC command"
        i2c_write(0x33);
        i2c_write(0x00);
        _delay_ms(80);                      // "Wait for 80ms to wait for the measurement to be completed"
        i2c_start();
        i2c_write(SLA_R);
        for (uint8_t i = 0; i < 7; i++) {   // Read 6 bytes (sending ACK after each)
            i2c_read();
            data[i] = TWDR;                 // Store received byte
        }
        for (uint8_t i = 0; i < 7; i++)
            print_hex_value(data[i]);
        uart_printstr("\r\n");
        i2c_stop();
    }
    return (0);
}
