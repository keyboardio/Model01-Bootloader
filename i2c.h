#pragma once

#include <stdint.h>

void i2c_init(void);
void i2c_send( uint8_t address, uint8_t * data, uint8_t length );

