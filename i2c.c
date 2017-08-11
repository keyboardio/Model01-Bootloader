#include <i2c.h>
#include <avr/io.h>
#include <util/delay.h>
#define TWI_START_TRANSMITTED     0x08
#define TWI_SLAW_ACKED            0x18
#define TWI_TXDATA_ACKED          0x28
#define TWI_TXDATA_NACKED         0x30


void i2c_init( void ) {
    // Init TWI as master.
    TWCR = _BV(TWEN);
    TWBR = 16; // 250bps @ 8MHz.
}

inline uint8_t i2c_send_one (uint8_t data, uint8_t response) {
    TWDR = data;
    TWCR = _BV(TWINT) | _BV(TWEN);
    do {} while ((TWCR & _BV(TWINT)) == 0);
    return (TWSR != response);
}

uint8_t i2c_send( uint8_t address, uint8_t *data, uint8_t length ) {
    uint8_t error = 0;
    _delay_ms(1);
    // START condition.
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    do {} while ((TWCR & _BV(TWINT)) == 0);
    error = (TWSR != TWI_START_TRANSMITTED);

    // Send SLA+W.
    if (!error) {
        error = i2c_send_one(((address & ~0x01) | 0x00), TWI_SLAW_ACKED);
    }

    // Send data bytes minus the last one. Expect ACK.
    const uint8_t * bufferPtr = data;
    if (!error) {
        for (uint8_t i = (length-1); i >0 ; --i) {
            error = i2c_send_one(*bufferPtr,TWI_TXDATA_ACKED);
            if (!error) {
                ++bufferPtr;
            } else {
                break;
            }
        }
    }

    // Send last data byte.
    if (!error) {
        error = i2c_send_one(*bufferPtr,TWI_TXDATA_NACKED);
    }

    // If there was an error, abort the communication
    // If the last byte got sent successfully, expect a NAK
    // Both of thse are the same signal
    TWCR = _BV(TWSTO) | _BV(TWINT) | _BV(TWEN);

    return (!error);
}
