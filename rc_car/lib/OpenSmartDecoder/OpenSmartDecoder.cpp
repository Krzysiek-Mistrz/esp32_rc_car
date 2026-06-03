#include "OpenSmartDecoder.h"

static const uint8_t symbols[] = {
    0x0d, 0x0e, 0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34
};

OpenSmartDecoder::OpenSmartDecoder(int rxPin) : 
    _rxPin(rxPin), _shiftReg(0), _synced(false), _bitCount(0), 
    _msgPtr(0), _expectedLen(0), _lastTime(0), _lastState(-1) {
    _currentData = {504, 520, 0, false};
}

void OpenSmartDecoder::begin() {
    pinMode(_rxPin, INPUT);
}

uint8_t OpenSmartDecoder::decode6bit(uint8_t s) {
    for (uint8_t i = 0; i < 16; i++) {
        if (symbols[i] == s) return i;
    }
    return 0xFF;
}

void OpenSmartDecoder::processBit(uint8_t bit) {
    _shiftReg = (_shiftReg >> 1) | (bit ? 0x800 : 0);
    
    if (!_synced) {
        if (_shiftReg == 0xb38) {
            _synced = true;
            _bitCount = 0;
            _msgPtr = 0;
            _expectedLen = 0;
        }
    } else {
        _bitCount++;
        if (_bitCount == 12) {
            uint8_t n1 = decode6bit(_shiftReg & 0x3f);
            uint8_t n2 = decode6bit(_shiftReg >> 6);
            
            if (n1 == 0xFF || n2 == 0xFF) {
                _synced = false;
                return;
            }
            
            uint8_t val = (n1 << 4) | n2;
            if (_msgPtr == 0) {
                _expectedLen = val;
                if (_expectedLen < 4 || _expectedLen > 15) {
                    _synced = false;
                    return;
                }
            }
            
            if (_msgPtr < sizeof(_msg)) _msg[_msgPtr++] = val;
            _bitCount = 0;

            if (_msgPtr >= _expectedLen && _expectedLen > 0) {
                if (_msgPtr >= 5 && _msg[1] == 0x7E) {
                    _currentData.x = _msg[2] << 2;
                    _currentData.y = _msg[3] << 2;
                    _currentData.button = _msg[4] & 0x07;
                    _currentData.updated = true;
                }
                _synced = false;
            }
        }
    }
}

bool OpenSmartDecoder::update() {
    int state = digitalRead(_rxPin);
    bool newData = false;

    if (state != _lastState) {
        unsigned long now = micros();
        unsigned long duration = now - _lastTime;
        _lastTime = now;
        
        int bits = (duration + 250) / 500;
        if (bits > 0 && bits < 15) {
            uint8_t bit_val = (_lastState == HIGH) ? 1 : 0;
            for (int i = 0; i < bits; i++) processBit(bit_val);
        }
        _lastState = state;
    }

    if (_currentData.updated) {
        _currentData.updated = false;
        newData = true;
    }
    return newData;
}

JoystickData OpenSmartDecoder::getData() {
    return _currentData;
}
