#ifndef TM1638_H
#define TM1638_H

#include "mbed.h"

class TM1638 {
public:
    TM1638(PinName stb, PinName clk, PinName dio);

    void sendCommand(uint8_t cmd);
    void sendData(uint8_t address, uint8_t data);
    void clearDisplay();

    void clear();                       // limpiar display
    void show(const char* str);         // mostrar string (0-9, A-F y espacio)
    void setBrightness(uint8_t level);  // 0..7
    uint8_t readKeys();                 // leer S1..S8 (bits 0..7)

private:
    DigitalOut _stb;
    DigitalOut _clk;
    DigitalInOut _dio;

    void start();
    void stop();
    void writeByte(uint8_t data);
    uint8_t readByte();
};

#endif





