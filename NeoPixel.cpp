#include "NeoPixel.h"

NeoPixel::NeoPixel(PinName mosi, uint8_t num_leds) 
    : _spi(mosi, NC, NC), _num_leds(num_leds) {
    
    // Configurar SPI para 3.2MHz (timing para WS2812B)
    _spi.format(8, 0);
    _spi.frequency(3200000);
    
    clear();
}

void NeoPixel::setPixelColor(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    if (index < _num_leds) {
        _pixels[index][0] = red;
        _pixels[index][1] = green;
        _pixels[index][2] = blue;
    }
}

void NeoPixel::clear() {
    for (uint8_t i = 0; i < _num_leds; i++) {
        _pixels[i][0] = 0;
        _pixels[i][1] = 0;
        _pixels[i][2] = 0;
    }
    show();
}

void NeoPixel::show() {
    createSPIBuffer();
    
    // Calcular número de bytes a enviar
    int total_bytes = _num_leds * 24;

    for (int i = 0; i < total_bytes; i++) {
        _spi.write(_spi_buffer[i]);
    }
    
    // Delay de reset requerido por WS2812B
    wait_us(80);
}

void NeoPixel::createSPIBuffer() {
    int buffer_index = 0;
    
    for (uint8_t led = 0; led < _num_leds; led++) {
        // Para cada LED, procesar Green, Red, Blue (orden GRB)
        uint8_t colors[3] = {_pixels[led][1], _pixels[led][0], _pixels[led][2]};
        
        for (uint8_t color_index = 0; color_index < 3; color_index++) {
            uint8_t color = colors[color_index];
            
            for (int8_t bit = 7; bit >= 0; bit--) {
                if (color & (1 << bit)) {
                    _spi_buffer[buffer_index++] = 0x06; // bit 1 → 110
                } else {
                    _spi_buffer[buffer_index++] = 0x04; // bit 0 → 100
                }
            }
        }
    }
}
