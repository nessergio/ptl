# Pick-to-Light (PTL) System

A hardware-based pick-to-light and sorting system designed for warehouse operations. The system uses RGB LEDs to highlight storage locations, helping workers efficiently collect orders or sort packages.

## Overview

This ESP32-based device connects to RGB LED indicators and a Bluetooth barcode scanner to create an intelligent warehouse assistance system. When a barcode is scanned, the corresponding shelf location lights up, guiding workers to the correct pick or sort location.

## Features

- **Barcode Scanner Integration**: Connects to BLE barcode scanners
- **RGB LED Control**: Supports up to 48 individual LED zones via CH423 I/O expanders
- **Additional NeoPixel Support**: 60 addressable RGB LEDs for visual feedback
- **WiFi Connectivity**: Operates in both Station (client) and AP (access point) modes
- **Web Interface**: Browser-based configuration and monitoring
- **REST API**: Programmatic control via HTTP endpoints
- **Real-time Updates**: Server-sent events for live status monitoring
- **Flexible Configuration**: JSON-based configuration for easy customization

## Use Cases

1. **Order Picking (Pick-to-Light)**: Workers scan order items, and LEDs highlight the shelf locations where items should be collected
2. **Package Sorting**: Scan package barcodes to illuminate destination bins or shelves
3. **Inventory Management**: Guide workers to specific storage locations
4. **Assembly Operations**: Highlight component locations based on BOM (Bill of Materials)

## Hardware Requirements

### Main Components
- ESP32 Development Board (ESP32-DOIT-DevKit-V1 or compatible)
- CH423 I/O Expander (up to 2 units for 48 outputs)
- RGB LED strips or individual LEDs for shelf indicators
- WS2812B NeoPixel LED strip (60 LEDs, optional)
- Bluetooth LE Barcode Scanner

### Connections

#### CH423 I/O Expanders
- **Wire0** (I2C): SDA=GPIO21, SCL=GPIO22 (default ESP32 I2C)
- **Wire1** (I2C): SDA=GPIO18, SCL=GPIO19

#### NeoPixel Strip
- Data Pin: GPIO16

#### Pin Mapping
- Pins 0-7: CH423 #1 GPIO pins
- Pins 8-23: CH423 #1 GPO pins
- Pins 24-31: CH423 #2 GPIO pins
- Pins 32-47: CH423 #2 GPO pins
- Pin 48: Special - controls all LEDs simultaneously

## Software Setup

### Prerequisites
- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- Git (for cloning dependencies)

### Installation

1. **Clone the repository**
```bash
git clone <repository-url>
cd ptl
```

2. **Install dependencies**

Dependencies are automatically managed by PlatformIO:
- ESPAsyncWebServer
- NimBLE-Arduino (v1.4.0+)
- ArduinoJson
- Adafruit NeoPixel (v1.11.0+)

3. **Build and upload**
```bash
platformio run --target upload
```

4. **Upload filesystem data** (web interface and config files)
```bash
platformio run --target uploadfs
```

### Initial Configuration

1. **First Boot**: The device will create a WiFi access point
   - SSID: From `data/config.json` (default: "99")
   - Password: From `data/config.json`

2. **Connect to the device**
   - Connect to the WiFi AP
   - Open browser to `http://192.168.4.1` (default AP IP)

3. **Configure WiFi** (if using station mode)
   - Edit `data/config.json` before uploading
   - Set `standalone: false`
   - Configure your network SSID and password

## Configuration

### config.json

Located in `data/config.json`:

```json
{
    "standalone": false,
    "ssid": "YourNetworkName",
    "wifipass": "YourPassword",
    "cidr": "192.168.88.201/24",
    "gw": "192.168.88.1",
    "addr": "aa:a8:a2:15:78:d9",
    "pins": ["A", "B", "C"],
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

**Parameters:**
- `standalone`: `true` for AP mode, `false` for station mode
- `ssid`: WiFi network name (or AP name if standalone)
- `wifipass`: WiFi password
- `cidr`: Static IP configuration (station mode)
- `gw`: Gateway IP
- `addr`: Bluetooth MAC address of the barcode scanner
- `pins`: Array of pin names/labels
- `service`: BLE service UUID for barcode scanner
- `charact`: BLE characteristic UUID for barcode scanner notifications

### table.json

Maps barcodes to shelf locations (pin names):

```json
{
    "A": ["1234567890", "0987654321"],
    "B": ["1111111111", "2222222222"],
    "C": ["3333333333"]
}
```

Format: `{ "PinName": ["barcode1", "barcode2", ...] }`

## API Documentation

### REST Endpoints

#### GET /api/wifiInfo
Returns WiFi connection information.

**Response:**
```json
{
    "status": "ok",
    "ssid": "NetworkName",
    "gw": "192.168.1.1",
    "dns": "192.168.1.1",
    "cidr": "192.168.1.100/24"
}
```

#### POST /api/setDevice
Configure BLE scanner connection.

**Request:**
```json
{
    "address": "aa:a8:a2:15:78:d9",
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

#### POST /api/blink
Manually trigger LED blinking for a specific pin.

**Request:**
```json
{
    "pin": 5
}
```

#### POST /api/setLed
Set LED based on barcode (legacy).

**Request:**
```json
{
    "code": "1234567890"
}
```

#### POST /api/writeConfig
Update device configuration (requires reboot).

**Query Parameters:**
- `reboot=true`: Restart device after saving

**Request Body:** Complete config.json structure

### Server-Sent Events

#### /events
Real-time event stream for monitoring.

**Event Types:**

1. **status** - Device status updates
```json
{
    "status": 1,
    "r": 1234567890,
    "w": 1234567890,
    "devices": [
        {
            "address": "aa:a8:a2:15:78:d9",
            "service": "0000feea-0000-1000-8000-00805f9b34fb"
        }
    ]
}
```

2. **scan** - Barcode scan events
```json
{
    "code": "1234567890",
    "pin": "A",
    "t": 1234567890
}
```

## Usage

### Basic Operation

1. **Power on the device**
2. **Wait for BLE connection** to the barcode scanner (check status LED/logs)
3. **Scan a barcode** with the paired scanner
4. **LED illuminates** at the corresponding shelf location
5. **LED blinks for 10 seconds** then turns off

### LED Behavior

- **Blink Duration**: 10 seconds (configurable in code: `blinkDuration`)
- **Blink Period**: 1 second on/off cycle (configurable: `blinkPeriod`)
- **Active State**: LED is ON (LOW signal to CH423)
- **Inactive State**: LED is OFF (HIGH signal to CH423)

### Web Interface

Access the web interface at the device's IP address:
- **AP Mode**: `http://192.168.4.1`
- **Station Mode**: Check serial monitor for assigned IP or configure static IP

Features:
- View connected BLE devices
- Configure scanner pairing
- Test individual LEDs
- View scan history
- Update configuration

## Troubleshooting

### BLE Scanner Not Connecting

1. Check scanner Bluetooth MAC address in config
2. Verify scanner is in pairing/advertising mode
3. Check serial monitor for connection logs
4. Ensure correct service and characteristic UUIDs

### LEDs Not Working

1. Verify I2C connections to CH423 chips
2. Check I2C addresses are responding (serial monitor shows "Wire0/Wire1 not found!" if missing)
3. Test with pin 48 to light all LEDs: POST to `/api/blink` with `{"pin": 48}`
4. Verify LED wiring (LOW = ON, HIGH = OFF for CH423)

### WiFi Connection Issues

1. Check SSID and password in config.json
2. Try standalone (AP) mode first
3. Monitor serial output (115200 baud) for connection status
4. Ensure router supports 2.4GHz WiFi (ESP32 doesn't support 5GHz)

### Barcode Not Triggering LED

1. Verify barcode exists in `table.json`
2. Check pin name mapping in `config.json`
3. Monitor `/events` endpoint for scan events
4. Check serial monitor for "R: [barcode] = [pin]" messages

## Development

### Serial Monitor

Baud rate: 115200

**Key Log Messages:**
- `Running ble on core 0` - BLE task started
- `Running blink on core 0` - LED control task started
- `CONNECTED TO DEVICE` - Scanner connected
- `SUBSCRIBED` - Listening for scanner notifications
- `R: [code] = [pin]` - Barcode processed
- `Starting to blink [pin]` - LED activated

### Code Structure

```
src/
├── main.cpp          # Main application, web server, WiFi
├── ble.cpp           # Bluetooth LE scanner connection
├── blink.cpp         # LED control via CH423
├── log.cpp           # Logging functionality
├── ptl.hpp           # Common definitions and structures
├── DFRobot_CH423.cpp # CH423 driver
└── DFRobot_CH423.h   # CH423 driver header

data/
├── index.html        # Web interface
├── config.json       # Device configuration
├── table.json        # Barcode to pin mapping
└── *.png            # Web interface icons
```

### Customization

**Change LED Blink Duration:**
Edit `src/blink.cpp`:
```cpp
unsigned long blinkDuration = 10000UL;  // milliseconds
```

**Change Blink Pattern:**
Edit `src/blink.cpp`:
```cpp
int blinkPeriod = 1000;  // Full cycle duration (ms)
int blinkFill = 500;     // ON duration (ms)
```

**Change BLE Scan Interval:**
Edit `src/ble.cpp`:
```cpp
#define SCAN_TIME 5  // seconds
```

## PCB Files

Pre-designed PCB files are included in the `pcb/` directory:
- Gerber files for manufacturing
- BOM (Bill of Materials)
- Pick-and-place files

Two PCB variants:
- `ptl-regi2`: Main board with CH423 support
- `ptl-mk_2`: Alternative design

## License

[Specify your license here]

## Contributing

[Specify contribution guidelines here]

## Support

For issues, questions, or contributions, please [specify contact method or issue tracker].
