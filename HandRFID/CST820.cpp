#include "CST820.h"

CST820::CST820(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin) {
    _sda  = sda_pin;
    _scl  = scl_pin;
    _rst  = rst_pin;
    _int  = int_pin;
    _wire = nullptr;
}

void CST820::begin(TwoWire &wire) {
    _wire = &wire;

    // Caller is expected to have already called wire.begin() —
    // we skip re-init here to avoid disturbing a bus that was
    // already set up (e.g. after an I2C probe).

    // Int pin: pulse high->low
    if (_int != -1) {
        pinMode(_int, OUTPUT);
        digitalWrite(_int, HIGH);
        delay(1);
        digitalWrite(_int, LOW);
        delay(1);
    }

    // Reset pin: pulse low->high
    if (_rst != -1) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(300);
    }

    // Disable auto low-power mode
    i2c_write(0xFE, 0xFF);
}

bool CST820::getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture) {
    bool fingerDown = (bool)i2c_read(0x02);

    *gesture = i2c_read(0x01);

    uint8_t data[4];
    i2c_read_continuous(0x03, data, 4);
    *x = ((data[0] & 0x0f) << 8) | data[1];
    *y = ((data[2] & 0x0f) << 8) | data[3];

    return fingerDown;
}

uint8_t CST820::i2c_read(uint8_t addr) {
    uint8_t rdData = 0;
    uint8_t rdDataCount;
    do {
        _wire->beginTransmission(I2C_ADDR_CST820);
        _wire->write(addr);
        _wire->endTransmission(false);
        rdDataCount = _wire->requestFrom(I2C_ADDR_CST820, 1);
    } while (rdDataCount == 0);
    while (_wire->available())
        rdData = _wire->read();
    return rdData;
}

uint8_t CST820::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length) {
    _wire->beginTransmission(I2C_ADDR_CST820);
    _wire->write(addr);
    if (_wire->endTransmission(true)) return -1;
    _wire->requestFrom(I2C_ADDR_CST820, length);
    for (uint32_t i = 0; i < length; i++)
        *data++ = _wire->read();
    return 0;
}

void CST820::i2c_write(uint8_t addr, uint8_t data) {
    _wire->beginTransmission(I2C_ADDR_CST820);
    _wire->write(addr);
    _wire->write(data);
    _wire->endTransmission();
}
