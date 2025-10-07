#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include "mbed.h"

class NeoPixel {
public:
    NeoPixel(PinName mosi, uint8_t num_leds);

    void setPixelColor(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
    void clear();
    void show();

private:
    SPI _spi;
    uint8_t _num_leds;
    uint8_t _pixels[60][3];     // hasta 60 LEDs máximo
    uint8_t _spi_buffer[60 * 24]; // 24 bits por LED (GRB)

    void createSPIBuffer();
};

#endif
