#include <Arduino.h>

const int RX_PIN = 19;

// Symbole 4-to-6 bitowe protokołu VirtualWire
const uint8_t symbols[] = {
    0x0d, 0x0e, 0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34
};

uint8_t decode_6bit(uint8_t s) {
    for (uint8_t i = 0; i < 16; i++) {
        if (symbols[i] == s) return i;
    }
    return 0xFF;
}

uint16_t shift_reg = 0;
bool synced = false;
uint8_t bit_count = 0;
uint8_t msg[15];
uint8_t msg_ptr = 0;
uint8_t expected_len = 0;

void process_bit(uint8_t bit) {
    shift_reg = (shift_reg >> 1) | (bit ? 0x800 : 0);
    
    if (!synced) {
        // Szukanie preambuły VirtualWire (0xB38)
        if (shift_reg == 0xb38) {
            synced = true;
            bit_count = 0;
            msg_ptr = 0;
            expected_len = 0;
            Serial.print("\n[SYNC] ");
        }
    } else {
        bit_count++;
        if (bit_count == 12) { // 12 bitów to jeden bajt danych (2x6bit)
            uint8_t n1 = decode_6bit(shift_reg & 0x3f); // Pierwszy odebrany symbol (High Nybble)
            uint8_t n2 = decode_6bit(shift_reg >> 6);   // Drugi odebrany symbol (Low Nybble)
            
            if (n1 == 0xFF || n2 == 0xFF) {
                synced = false;
                return;
            }
            
            // VirtualWire wysyła High Nybble przed Low Nybble
            uint8_t val = (n1 << 4) | n2;
            Serial.printf("[%02X]", val);
            
            if (msg_ptr == 0) {
                expected_len = val;
                // Sensowna długość dla Open-Smart to ok. 8 bajtów
                if (expected_len < 4 || expected_len > 15) {
                    synced = false;
                    return;
                }
            }
            
            if (msg_ptr < sizeof(msg)) msg[msg_ptr++] = val;
            bit_count = 0;

            if (msg_ptr >= expected_len && expected_len > 0) {
                // Dekodowanie ramki joysticka [Len, 7E, X, Y, Z+Btn, CRC, CRC]
                if (msg_ptr >= 5 && msg[1] == 0x7E) {
                    int xVal = msg[2] << 2;
                    int yVal = msg[3] << 2;
                    uint8_t btn = msg[4] & 0x07;
                    Serial.printf(" -> JOYSTICK: X:%d Y:%d Btn:%d", xVal, yVal, btn);
                }
                synced = false;
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    delay(10000); // Twoj wazny delay
    pinMode(RX_PIN, INPUT);
    Serial.println("\n--- DEKODER V3 (D19) ---");
    Serial.println("Czekam na dane...");
}

void loop() {
    static unsigned long last_time = 0;
    static int last_state = -1;

    int state = digitalRead(RX_PIN);
    if (state != last_state) {
        unsigned long now = micros();
        unsigned long duration = now - last_time;
        
        // Obliczanie bitów z tolerancją na asymetrię sygnału
        int bits = (duration + 250) / 500;
        
        if (bits > 0 && bits < 15) {
            uint8_t bit_val = (last_state == HIGH) ? 1 : 0;
            for (int i = 0; i < bits; i++) process_bit(bit_val);
        }
        
        last_time = now;
        last_state = state;
    }
}
