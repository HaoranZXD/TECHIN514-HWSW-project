# "AtmosTrack" Personal Weather Station

## Overview

"AtmosTrack" provides localized weather monitoring by collecting atmospheric data in real-time. It's designed for domestic use, featuring an outdoor sensor unit that is compact and waterproof. The indoor display boasts an intuitive gauge and vibrant LED indicators, all encapsulated within a modern design.

![General Sketch](/General%20Sketch.jpg)

## Sensing Device - "AtmosSensor"

![Detailed Sketch of Sensing Device](/AtmosSensor.png)

The "AtmosSensor" outdoor unit is outfitted with the HTU21D sensor for accurate humidity and temperature measurements, alongside the CCS811 sensor for CO2 and VOC levels. These are managed by the ESP32 processor. Data is wirelessly transmitted to the indoor unit via Wi-Fi. The unit is powered by a LP323450 rechargeable lithium polymer ion battery pack, ensuring continuous operation.

## Display Device - "AtmosDisplay"

![Detailed Sketch of Display Device](/AtmosDisplay.png)

The indoor "AtmosDisplay" presents the weather data on a ST7735R TFT LCD and utilizes a custom gauge with a stepper motor (28BYJ-48) for an analog display. It's powered by an ESP32 processor, featuring Wi-Fi connectivity for real-time data updates. User interaction is facilitated through a capacitive touch button (TTP223). The display unit is powered by a 18650 lithium-ion battery, providing portability and easy recharging.

## Communication and System Diagrams

![Communication and System Diagrams](/Communication%20and%20System%20Diagrams.png)



The "AtmosSensor" and "AtmosDisplay" units communicate wirelessly through Wi-Fi, ensuring a consistent stream of data. The ESP32 of "AtmosSensor" is responsible for gathering sensor data and performing initial processing before transmission. The ESP32 of "AtmosDisplay" processes the data upon receipt and updates the display accordingly, offering a dynamic and interactive way to visualize weather data.

## Datasheets Folder

- `[HTU21D](datasheets/HTU21D.pdf)`
- `[CCS811](datasheets/CCS811.pdf)`
- `[ESP32](datasheets/ESP32-S3.pdf)`
- `[28BYJ-48](datasheets/28BYJ-48.pdf)`
- `[ST7735R](datasheets/ST7735R.pdf)`
- `[TTP223](datasheets/TTP223.pdf)`
- `[ICR18650](datasheets/ICR18650.pdf)`
- `[LP323450](datasheets/LP323450.pdf)`


