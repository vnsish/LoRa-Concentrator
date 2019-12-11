# LoRa Concentrator
LoRa concentrator project developed for my computer engineering thesis. Uses the [pySX127x](https://github.com/mayeranalytics/pySX127x) interface by mayeranalytics. Built using a Raspberry Pi 3 Model B+ and a HopeRF RF95W LoRa module.

## Pinout
The pins on the RPi are using the physical board numbering.

| RFM95W | RPi 3 B+       |
|--------|----------------|
| GND    | P25 (GND)      |
| MISO   | P21 (SPI MISO) |
| MOSI   | P19 (SPI MOSI) |
| SCK    | P23 (SPI CLK)  |
| NSS    | P24 (SPI CE0)  |
| DIO0   | P22 (GPIO 25)  |
| DIO1   | P18 (GPIO 24)  |
| 3.3V   | P17 (3.3V)     |

## Message Format
The concentrator receives messages in this format: 

> NODE_ID;API_KEY;Value1;Value2;...;ValueN

The service being used for this project is [ThingSpeak](https://thingspeak.com/).
