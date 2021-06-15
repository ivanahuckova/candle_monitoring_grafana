# Candle monitoring with Arduino and Grafana

The candle monitoring system helps you learn more about your candle burning activities and lets you extiguish your candle directly from Grafana. It monitors if candle's flame and released PM particles.

This project was part of **[GrafanaCONline 2021 Easy DIY IoT projects with Grafana](https://grafana.com/go/grafanaconline/2021/diy-iot/)** session.

<img src="imgs/parts1.JPG" width="700" title="Image of how the system looks">
<img src="imgs/parts2.JPG" width="700" title="Image of how the system looks">

## Arduino & Grafana

To start, review and follow steps in the **[Grafana DIY IoT repo](https://github.com/grafana/diy-iot)**.

## Hardware

To build this system, you will need:

- **1 [Arduino MKR Wifi 1010](https://store.arduino.cc/arduino-mkr-wifi-1010)** development board
- **1 [Waveshare Sharp GP2Y1010AU0F](https://www.waveshare.com/dust-sensor.htm)** sensor for PM paricles
- **1 [Flame sensor](https://www.electronicshub.org/arduino-flame-sensor-interface/)** to detect flame
- **1[L298N DC motor driver](https://lastminuteengineers.com/l298n-dc-stepper-driver-arduino-tutorial/)** to control DC motor
- **1 DC motor**
- **1 9V battery**
- **1 9V battery clip**
- **1 micro USB cable**
- **1 USB charger**
- **F-F, M-F && M-M Dupont cables**

Plus:

- **Box or stand** to put this all together and build the monitoring system

<img src="imgs/developing.JPG" width="700" title="Image of hardware">

## Libraries:

- **ArduinoBearSSL** by Arduino
- **ArduinoHttpClient** by Arduino
- **PrometheusArduino** by Ed Welch
- **PromLokiTransport** by Ed Welch
- **SnappyProto** by Ed Welch

## Circuit & Wiring diagrams

<img src="imgs/wire_diagram.png" width="700" title="Wire diagram">

## Software

Download this repo that includes the software for room comfort monitoring. Update **config.h** file with your names, passwords and API keys. Upload software to your board using Arduino IDE.

## Extinguishing candle from Grafana

This repo contains simple server that stores information if the lid that puts of the candle has been closed or open. This can be simply hosted trough [Heroku platform](https://dashboard.heroku.com/) or whatever you prefer. In config.h add the client and secret (if secret is used).

In Grafana, use the [Button panel](https://grafana.com/grafana/plugins/cloudspout-button-panel/) to send request to open/close the candle monitoring.
