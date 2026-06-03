# RC Car ESP32 - Open-Smart Wireless Control

Projekt zdalnie sterowanego samochodu opartego na **ESP32**, wykorzystujący bezprzewodowy joystick **Open-Smart** (częstotliwość 315/433MHz) oraz sterownik silników **TB6612FNG**.

## Architektura Systemu

### 1. Dekodowanie sygnału (`OpenSmartDecoder`)
Biblioteka znajdująca się w `rc_car/lib/OpenSmartDecoder` odpowiada za niskopoziomowe dekodowanie sygnału z modułu radiowego. 
- **Metoda:** Analizuje zmiany stanów na pinie RX (Manchester encoding / Custom pulse width modulation).
- **Dane:** Zwraca strukturę `JoystickData` zawierającą wartości osi X, Y oraz stan przycisków (zakres 0-1020).

### 2. Sterowanie (`main.cpp`)
Główny program realizuje logikę:
- **Serwo (Skręt):** Mapuje oś X joysticka na kąt wychylenia serwa (standardowo 45° - 135°).
- **Silniki (Napęd):** Mapuje oś Y na sygnał PWM dla sterownika TB6612FNG.
- **Diagnostyka:** Wyświetla na Serial Monitorze aktywność pinu RX oraz przetworzone dane.

## Diagnoza problemów z napędem (Silniki RS390)

Jeśli Twoje silniki **RS390** tylko drgają i piszczą, problemem jest **napięcie i prąd**:
1. **TB6612FNG jest za słaby:** Silniki RS390 przy 12V (3S) mogą pobierać prąd rzędu kilku amperów pod obciążeniem. TB6612 obsługuje tylko **1.2A na kanał**. Sterownik przegrzewa się i odcina zasilanie niemal natychmiast.
2. **Rozwiązanie:** Należy wymienić sterownik na mocniejszy, np. **BTS7960 (43A)** lub **VNH5019**.
3. **Piszczenie:** To dźwięk częstotliwości PWM. Silnik dostaje za mało prądu, by ruszyć, ale uzwojenia wibrują słyszalnie.

## Konfiguracja Pinów (ESP32)

| Funkcja | Pin ESP32 |
| :--- | :--- |
| **RX (Radio)** | 19 |
| **Servo (Skręt)** | 14 |
| **PWMA (Silnik L)** | 25 |
| **AIN1 / AIN2** | 26 / 27 |
| **PWMB (Silnik R)** | 32 |
| **BIN1 / BIN2** | 33 / 21 |
| **STBY (Driver)** | 15 |

## Struktura folderów
- `rc_car/` - Główny projekt PlatformIO (kod źródłowy, biblioteki).
- `old/` - Archiwalna dokumentacja, schematy i stare wersje oprogramowania dla ATmega8A.

---