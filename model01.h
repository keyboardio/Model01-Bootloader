#pragma once


#include "i2c.h"

#define ATTINY_I2C_ADDR 0xB0
#define BLUE 0xFF,0x00,0x00
#define RED 0x00, 0x00, 0xFF
#define UPDATE_LED_CMD 0x04
static uint8_t make_leds_black[] = {0x03,0x00,0x00,0x00};

static uint8_t progress_led = 24; // This is the LED on the "prog" key, bitshifted by 3 bits
static inline void CheckReprogrammingKey(void) {

    // Hold the ATTiny in reset, so it can't mess with this read
    DDRC |= _BV(6);
    PORTC &= ~_BV(6);

    // Light up PF0 and read PF1

    DDRF &= ~_BV(1); // make the col pin an input
    DDRF |= _BV(0); // make the row pin an output
    PORTF |= _BV(1); // turn on pullup
    PORTF &= ~_BV(0); // make our output low

    // we need a moment to get the read, or we get some real weird behavior
    // specifically, at random intervals, we end up in the bootloader on first boot
    // even if the prog key isn't held down.
    //
    // I (jesse) believe that the issue is that we were getting our reads
    // before the ATTiny had fully get reset
    _delay_ms(5);
    if ( PINF & _BV(1)) { // If the pin is hot
        _delay_ms(10); // debounce
        if (PINF & _BV(1)) { // If it's still hot, no key was pressed
            // Start the sketch
            PORTC |= _BV(6); // Turn the ATTiny back on
            StartSketch();
        }
    }
    PORTC |= _BV(6); // Turn the ATTiny back on
}


static inline void UpdateProgressLED(void) {
    // We bitshift the LED counter by 3 to slow it down a bit
    uint8_t led_cmd[] = { UPDATE_LED_CMD, progress_led>>3, RED };
    if (progress_led >= 256) {
        progress_led = 0;
    }
    i2c_send(ATTINY_I2C_ADDR, &led_cmd[0], sizeof(led_cmd));
}

static inline void EnableLEDs(void) {
    // Turn on power to the LED net
    DDRC |= _BV(7);
    PORTC |= _BV(7);
}

__attribute__ ((noinline)) static void TurnLEDsOff(void) {
    i2c_send( ATTINY_I2C_ADDR, &make_leds_black[0], sizeof(make_leds_black));

}


void InitLEDController() {
i2c_init();

    // Set the LEDs to black, so they don't flash.
    TurnLEDsOff();

}
    
