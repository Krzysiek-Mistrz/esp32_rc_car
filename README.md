# RC Car ESP32 - Open-Smart Wireless Control

This project is an ESP32-based remote-controlled car using the Open-Smart Wireless Joystick (315/433MHz) and the TB6612FNG motor driver.

## System Architecture

### 1. Signal Decoding (OpenSmartDecoder)
The library located in 'rc_car/lib/OpenSmartDecoder' handles low-level decoding of the radio module signal.
- Method: Analyzes state changes on the RX pin (Manchester encoding / Custom pulse width modulation).
- Data: Returns a 'JoystickData' structure containing X, Y axis values and button states (range 0-1020).

### 2. Control Logic (main.cpp)
The main program implements the following:
- Steering (Servo): Maps the joystick X-axis to the servo angle (standard 45 - 135 degrees).
- Drive (Motors): Maps the Y-axis to PWM signals for the TB6612FNG driver.
- Diagnostics: Outputs RX pin activity and processed data to the Serial Monitor.

## Drive System Diagnosis (RS390 Motors)

If the RS390 motors only jitter and whine, the issue is related to voltage and current:
1. TB6612FNG Limitation: RS390 motors at 12V (3S) can draw several amperes under load. The TB6612 only supports 1.2A per channel. The driver overheats and cuts power almost immediately.
2. Solution: Replace the driver with a more powerful one, such as BTS7960 (43A) or VNH5019.
3. Whining: This is the sound of the PWM frequency. The motor receives enough voltage to vibrate the windings but not enough current to overcome static friction.

## Pin Configuration (ESP32)

| Function | ESP32 Pin |
| :--- | :--- |
| RX (Radio) | 19 |
| Servo (Steering) | 14 |
| PWMA (Motor L) | 25 |
| AIN1 / AIN2 | 26 / 27 |
| PWMB (Motor R) | 32 |
| BIN1 / BIN2 | 33 / 21 |
| STBY (Driver) | 15 |

## Directory Structure
- rc_car/ - Main PlatformIO project (source code, libraries).
- old/ - Archival documentation, schematics, and old firmware versions for ATmega8A.

## License
This project is licensed under the Creative Commons Attribution 4.0 International (CC BY 4.0) license.
