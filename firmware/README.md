# Firmware do ESP32

Este código realiza a leitura dos sensores e envia via JSON para o servidor Flask.

## 🔌 Pinagem (Pinout)

| Componente | Pino ESP32 | Pino Módulo |
| :--- | :--- | :--- |
| **I2C (BMP/HTU)** | GPIO 21 | SDA |
| **I2C (BMP/HTU)** | GPIO 22 | SCL |
| **LDR** | GPIO 34 | Analog Out |

## Bibliotecas Necessárias
- Adafruit BMP085 Library
- Adafruit HTU21DF Library
- ArduinoJson (v6)
