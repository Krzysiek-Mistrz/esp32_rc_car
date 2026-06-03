# RC Car ESP32 - Open-Smart Wireless Control

This project is an ESP32-based remote-controlled car using the Open-Smart Wireless Joystick (315/433MHz) and the BTS7960 43A High-Power motor driver.

## System Architecture

### 1. Signal Decoding (OpenSmartDecoder)
The library located in 'rc_car/lib/OpenSmartDecoder' handles low-level decoding of the radio module signal.
- Method: Analyzes Manchester-encoded pulses from the radio receiver.
- Data: Returns X, Y axis values and button states.

### 2. Control Logic (main.cpp)
- Steering: Servo-based steering on the front axle.
- Drive: BTS7960 driver controlling two RS390 rear motors in parallel.

## Pin Configuration (ESP32)

| Function | ESP32 Pin | BTS7960 Pin |
| :--- | :--- | :--- |
| **RX (Radio)** | 19 | - |
| **Servo (Steering)** | 14 | - |
| **Forward PWM** | 25 | RPWM |
| **Reverse PWM** | 26 | LPWM |
| **Driver Enable** | 15 | R_EN + L_EN |

## Motor Driver: BTS7960 (43A)
The transition to BTS7960 was necessary to handle the high current requirements of the RS390 motors when powered by a 3S (12V) battery.
- Logic VCC: 5V (from ESP32)
- Common GND: Connect ESP32 ground and Battery ground.
- Power: Connect 12V Battery to B+ and B-.

## License
This project is licensed under the Creative Commons Attribution 4.0 International (CC BY 4.0) license.
