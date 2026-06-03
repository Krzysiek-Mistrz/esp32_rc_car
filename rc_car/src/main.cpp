#include <Arduino.h>
#include <ESP32Servo.h>
#include "OpenSmartDecoder.h"

// --- PIN CONFIGURATION (BTS7960) ---
const int RPWM = 25;   // forward pwm
const int LPWM = 26;   // reverse pwm
const int EN_PIN = 15; // enable pin (combined r_en and l_en)

const int SERVO_PIN = 14;
const int RX_PIN = 19;

// --- OBJECTS ---
Servo steeringServo;
OpenSmartDecoder decoder(RX_PIN);

// --- DIAGNOSTICS ---
long pulseCount = 0;
int lastRxState = -1;

void setMotors(int speed) {
    // enable the driver
    digitalWrite(EN_PIN, HIGH);
    
    speed = constrain(speed, -255, 255);
    
    // dead zone
    if (abs(speed) < 30) {
        analogWrite(RPWM, 0);
        analogWrite(LPWM, 0);
        return;
    }

    if (speed > 0) { // forward
        analogWrite(RPWM, abs(speed));
        analogWrite(LPWM, 0);
    } else { // reverse
        analogWrite(RPWM, 0);
        analogWrite(LPWM, abs(speed));
    }
}

void setup() {
    // keep motors off at startup
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW);
    
    Serial.begin(115200);
    while (!Serial);
    delay(1000); 

    decoder.begin();
    
    steeringServo.attach(SERVO_PIN);
    steeringServo.write(90); // straight
    
    pinMode(RPWM, OUTPUT);
    pinMode(LPWM, OUTPUT);
    
    Serial.println("\n============================");
    Serial.println("   rc car system startup   ");
    Serial.println("============================");
}

void loop() {
    // --- step 1: pulse counter (hardware diagnostics) ---
    int s = digitalRead(RX_PIN);
    if (s != lastRxState) {
        pulseCount++;
        lastRxState = s;
    }

    // --- step 2: radio decoding ---
    if (decoder.update()) {
        JoystickData data = decoder.getData();
        
        // steering (servo)
        int angle = map(data.x, 0, 1020, 135, 45);
        steeringServo.write(angle);
        
        // drive control (throttle)
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
