#include "TM1638.h"

// Tabla segmentos para 0-9 y A-F (7-seg)
static const uint8_t SEG_MAP[16] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // b
    0x39, // C
    0x5E, // d
    0x79, // E
    0x71  // F
};

// ---------------- constructor ----------------
TM1638::TM1638(PinName stb, PinName clk, PinName dio)
    : _stb(stb, 1), _clk(clk, 1), _dio(dio)
{
    // Inicializar _dio como entrada sin pull
    _dio.input();
    _dio.mode(PullNone);

    // limpiamos
    clear();
}

// ---------------- low-level ----------------
void TM1638::start() {
    _stb = 0;
}

void TM1638::stop() {
    _stb = 1;
}

void TM1638::writeByte(uint8_t data) {
    // escribir LSB primero
    for (int i = 0; i < 8; i++) {
        _clk = 0;
        _dio.output();
        _dio = (data >> i) & 0x01;
        _clk = 1;
    }
}

uint8_t TM1638::readByte() {
    uint8_t val = 0;
    _dio.input();
    for (int i = 0; i < 8; i++) {
        _clk = 0;
        wait_us(1);
        if (_dio.read()) {
            val |= (1 << i);
        }
        _clk = 1;
    }
    return val;
}

// ---------------- comandos ----------------
void TM1638::sendCommand(uint8_t cmd) {
    start();
    writeByte(cmd);
    stop();
}

void TM1638::sendData(uint8_t address, uint8_t data) {
    sendCommand(0x44); // write to fixed address
    start();
    writeByte(0xC0 | address);
    writeByte(data);
    stop();
}

// brillo
void TM1638::setBrightness(uint8_t level) {
    if (level > 7) level = 7;
    sendCommand(0x88 | level); // display on + level
}

// clear
void TM1638::clear() {
    // escribe 0 en todas las posiciones (16 posiciones: digit+LED)
    sendCommand(0x40);      // modo auto-incremento
    start();
    writeByte(0xC0);        // dirección inicial
    for (int i = 0; i < 16; i++) writeByte(0x00);
    stop();
}

// ---------------- mostrar texto/numero ----------------
// convierte carácter aceptado a patrón 7-seg (soporta '0'-'9', 'A'-'F', espacio)
static uint8_t encode_char(char c) {
    if (c >= '0' && c <= '9') return SEG_MAP[c - '0'];
    if (c >= 'A' && c <= 'F') return SEG_MAP[10 + (c - 'A')];
    if (c >= 'a' && c <= 'f') return SEG_MAP[10 + (c - 'a')];
    return 0x00; // espacio o no soportado
}

void TM1638::show(const char* str) {
    // escribe los 8 dígitos en 7-seg
    sendCommand(0x40); // modo auto-incremento
    start();
    writeByte(0xC0);
    for (int i = 0; i < 8; i++) {
        char c = str[i];
        if (c == '\0') {
            writeByte(0x00);
        } else {
            uint8_t seg = encode_char(c);
            writeByte(seg);
        }
        writeByte(0x00); // byte asociado al LED superior (lo dejamos 0)
    }
    stop();
}

void TM1638::clearDisplay() {
    sendCommand(0x40); // modo auto-incremento
    start();

    // Borra todas las posiciones
    for (uint8_t i = 0; i < 16; i++) {
        sendData(i, 0x00); // dirección i, valor 0x00
    }

    stop();
}


// ---------------- lectura de teclas ----------------
// Devuelve un byte con bit0=S1 ... bit7=S8
uint8_t TM1638::readKeys() {
    uint8_t keys = 0;
    start();
    writeByte(0x42); // comando read key scan

    for (int i = 0; i < 4; i++) {
        uint8_t data = readByte();
        // data bit0 -> S(i+1) (S1..S4)
        if (data & 0x01) keys |= (1 << i);
        // data bit4 -> S(i+5) (S5..S8)
        if (data & 0x10) keys |= (1 << (i + 4));
    }

    stop();
    return keys;
}


