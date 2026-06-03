#ifndef OPEN_SMART_DECODER_H
#define OPEN_SMART_DECODER_H

#include <Arduino.h>

struct JoystickData {
    int x;
    int y;
    int button;
    bool updated;
};

class OpenSmartDecoder {
public:
    OpenSmartDecoder(int rxPin);
    void begin();
    bool update(); // Zwraca true, gdy odebrano nową paczkę
    JoystickData getData();

private:
    int _rxPin;
    uint16_t _shiftReg;
    bool _synced;
    uint8_t _bitCount;
    uint8_t _msg[15];
    uint8_t _msgPtr;
    uint8_t _expectedLen;
    JoystickData _currentData;
    unsigned long _lastTime;
    int _lastState;

    void processBit(uint8_t bit);
    uint8_t decode6bit(uint8_t s);
};

#endif
