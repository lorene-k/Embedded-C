#include <avr/io.h>
#include <util/twi.h>

#define UART_BAUDRATE 115200
#define round(x) (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5))     // Round to nearest int
#define MYUBRR round((F_CPU / (16.0 * UART_BAUDRATE)) - 1.0)    // Round to 8

#define SLA_ADDR 0x38
#define SLA_W (SLA_ADDR << 1) | 0   // 0x70 (8-bit address for write)
#define SLA_R ((SLA_ADDR << 1) | 1) // 0x71 (8-bit address for read)

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
        uart_printstr("Master Transmitter: Slave ACK");
    else if (status_code == TW_MT_SLA_NACK)
        uart_printstr("Master Transmitter : Slave NACK");
    else if (status_code == TW_MT_DATA_ACK)
        uart_printstr("Master Transmitter : Data ACK");
    else if (status_code == TW_MT_DATA_NACK)
        uart_printstr("Master Transmitter: Data NACK");
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

// ************************************************************** I2C SETUP */
void i2c_init(void) {
    TWBR = 72;                              // SCLfrequency (100000) = F_CPU / (16 + 2(TWBR) x prescaler)
}

void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA)      // Send start condition
        | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))          // Wait for TWI Interrupt Flag set = START transmitted
        ;
    print_status((TWSR & 0xF8));            // Check value of TWI status register (mask prescaler bits)
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN)       // Transmit STOP condition
        | (1 << TWSTO);
}

int main() {
    uart_init();
    i2c_init();
    i2c_start();
    i2c_stop();
    while (1)
        ;
    return (0);
}
