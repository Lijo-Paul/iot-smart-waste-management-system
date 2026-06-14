# IoT-Enabled and Automated Waste Management System

This project uses ESP32, Ultrasonic Sensor, IR Sensor, Servo Motor,
I2C LCD, Wi-Fi, and Firebase Realtime Database to create a smart
waste management system.

## Features
- Real-time bin level monitoring
- Automatic touch-free lid operation
- Firebase cloud integration
- LCD status display
- Full-bin alerts

## Hardware
- ESP32
- HC-SR04 Ultrasonic Sensor
- IR Sensor
- MG90S Servo Motor
- 16x2 I2C LCD

## Software
- Arduino IDE
- Firebase Realtime Database
- ESP32 Libraries

## Circuit Connections

### ESP32 Connections

| Component | ESP32 Pin |
|------------|------------|
| Ultrasonic TRIG | GPIO 5 |
| Ultrasonic ECHO | GPIO 18 |
| IR Sensor OUT | GPIO 19 |
| Servo Signal | GPIO 23 |
| LCD SDA | GPIO 21 |
| LCD SCL | GPIO 22 |

### Power Connections

| Component | Power |
|------------|------------|
| Ultrasonic Sensor | 5V |
| IR Sensor | 3.3V / 5V |
| Servo Motor (MG90S) | External 5V Supply |
| LCD I2C Module | 5V |
| ESP32 | USB 5V |

### Important Note

- The servo motor uses a separate 5V power supply.
- Servo GND and ESP32 GND must be connected together.
- A 470µF–1000µF capacitor is recommended across the servo power supply to reduce brownout resets.
- The HC-SR04 ECHO pin should be connected through a voltage divider before connecting to the ESP32.

## System Architecture

```text
User
  │
  ▼
IR Sensor ─────► ESP32 ◄───── Ultrasonic Sensor
                    │
                    │
                    ▼
              Servo Motor
            (Automatic Lid)
                    │
                    ▼
              I2C LCD Display
                    │
                    ▼
                 Wi-Fi
                    │
                    ▼
      Firebase Realtime Database
                    │
                    ▼
          Real-Time Monitoring
```

## Working Principle

1. Ultrasonic sensor continuously measures the waste level inside the bin.
2. IR sensor detects the presence of a user.
3. ESP32 processes the sensor data.
4. When a user approaches, the servo motor automatically opens the lid.
5. The LCD displays the current bin status.
6. Bin fill percentage and status are uploaded to Firebase through Wi-Fi.
7. When the bin becomes full, the system displays a warning and updates the cloud database.


## Authors
- Lijo Paul
