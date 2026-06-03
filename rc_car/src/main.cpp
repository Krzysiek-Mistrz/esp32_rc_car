#include <Arduino.h>
#include <ESP32Servo.h>
#include "OpenSmartDecoder.h"

// --- KONFIGURACJA PINÓW ---
const int PWMA = 25;
const int AIN1 = 26;
const int AIN2 = 27;
const int PWMB = 32;
const int BIN1 = 33;
const int BIN2 = 21;
const int STBY = 15;

const int SERVO_PIN = 14;
const int RX_PIN = 19;

// --- OBIEKTY ---
Servo steeringServo;
OpenSmartDecoder decoder(RX_PIN);

// --- ZMIENNE DIAGNOSTYCZNE ---
long pulseCount = 0;
int lastRxState = -1;

void setMotors(int speed) {
    // Aktywacja sterownika
    digitalWrite(STBY, HIGH);
    
    speed = constrain(speed, -255, 255);
    
    // Martwa strefa
    if (abs(speed) < 30) {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, LOW);
        analogWrite(PWMA, 0);
        analogWrite(PWMB, 0);
        return;
    }

    if (speed > 0) { // Przód
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
    } else { // Tył
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
    }
    
    analogWrite(PWMA, abs(speed));
    analogWrite(PWMB, abs(speed));
}

void setup() {
    // Najpierw zabezpieczamy silniki
    pinMode(STBY, OUTPUT);
    digitalWrite(STBY, LOW);
    
    Serial.begin(115200);
    while (!Serial);
    // Krótkie opóźnienie na stabilizację Seriala
    delay(10000); 

    decoder.begin();
    
    steeringServo.attach(SERVO_PIN);
    steeringServo.write(90); // Prosto
    
    pinMode(PWMA, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    
    Serial.println("\n============================");
    Serial.println("   rc car system startup   ");
    Serial.println("============================");
}

void loop() {
    // --- step 1: pulse counter (hardware diagnostics) ---
    // check if rx pin sees any state changes
    int s = digitalRead(RX_PIN);
    if (s != lastRxState) {
        pulseCount++;
        lastRxState = s;
    }

    // --- step 2: radio decoding ---
    // this function must be called as often as possible (no delays!)
    if (decoder.update()) {
        JoystickData data = decoder.getData();
        
        // steering (servo)
        int angle = map(data.x, 0, 1020, 135, 45);
        steeringServo.write(angle);
        
        // motor control (throttle)
        int speed = map(data.y, 0, 1020, -255, 255);
        setMotors(speed);
        
        Serial.printf("rx ok -> x:%4d (ang:%d) | y:%4d (spd:%d) | btn:%d\n", 
                      data.x, angle, data.y, speed, data.button);
    }

    // --- step 3: status report (every 2 seconds) ---
    static unsigned long lastDiag = 0;
    if (millis() - lastDiag > 2000) {
        Serial.printf("[status] rx activity: %ld changes/2s | uptime: %ld s\n", 
                      pulseCount, millis()/1000);
        pulseCount = 0;
        lastDiag = millis();
    }
}
