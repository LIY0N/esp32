# ESP32 RPM to CAN Bus Converter

This sketch reads RPM sensor signals via interrupts, processes them, and sends data over the CAN bus.

## Hardware Requirements

- ESP32-c3 (or compatible board)
- RPM sensor (encoder or pulse sensor)
- CAN transceiver (TJA1050)
- 3.3V/5v power supply

## Wiring

### RPM Sensor
- Connect the RPM sensor to pin 10 (GPIO10) on the ESP32

### CAN Bus
- CAN_TX (transmit) - GPIO3
- CAN_RX (receive) - GPIO2

Connect these pins to the corresponding CAN transceiver pins.

## Configuration

Main configurable parameters:

```cpp
const int rpmPin = 10;          // Pin for RPM sensor
const int timeoutValue = 10;    // Timeout for RPM reset
const int numReadings = 1;      // Array size for smoothing
int smoothing = 1;              // Enable/disable smoothing
