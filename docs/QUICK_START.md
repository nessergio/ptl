# Quick Start Guide

Get your Pick-to-Light system up and running in 15 minutes.

## Prerequisites

- Assembled hardware (see [HARDWARE_SETUP.md](HARDWARE_SETUP.md) if not complete)
- Computer with USB port
- PlatformIO installed
- Bluetooth barcode scanner

## Step 1: Flash the Firmware (5 minutes)

### 1.1 Connect ESP32 to Computer

Connect your ESP32 development board via USB cable.

### 1.2 Build and Upload Firmware

```bash
# Navigate to project directory
cd ptl

# Build and upload firmware
platformio run --target upload

# Upload web files and configuration
platformio run --target uploadfs
```

### 1.3 Monitor Serial Output

```bash
platformio device monitor

# Or specify baud rate manually
platformio device monitor --baud 115200
```

**Expected output:**
```
Running ble on core 0
Running blink on core 0
Init BLE ok
Setting AP (99)…
192.168.4.1
```

## Step 2: Initial Configuration (5 minutes)

### 2.1 Connect to Device WiFi

The device creates a WiFi access point on first boot.

**Default credentials:**
- SSID: `99` (from config.json)
- Password: `123123123` (from config.json)

Connect your computer or phone to this network.

### 2.2 Access Web Interface

Open browser and navigate to:
```
http://192.168.4.1
```

You should see the Pick-to-Light web interface.

### 2.3 Configure Your Network (Optional)

If you want the device to connect to your existing WiFi network:

1. Edit `data/config.json` before uploading:
   ```json
   {
       "standalone": false,
       "ssid": "YourWiFiName",
       "wifipass": "YourWiFiPassword",
       ...
   }
   ```

2. Re-upload filesystem:
   ```bash
   platformio run --target uploadfs
   ```

3. Restart the device

4. Check serial monitor for assigned IP address

## Step 3: Pair Bluetooth Scanner (3 minutes)

### 3.1 Find Scanner MAC Address

1. Power on your Bluetooth barcode scanner
2. Watch serial monitor output for:
   ```
   Scanning BLE...
   A: aa:a8:a2:15:78:d9 N: Scanner S: 0000feea-0000-1000-8000-00805f9b34fb RSSI: -45
   ```

3. Note the MAC address (e.g., `aa:a8:a2:15:78:d9`)

### 3.2 Configure Scanner in Web Interface

**Method 1: Using Web Interface**

1. Open device web page
2. Look for discovered BLE devices table
3. Click on your scanner device
4. Device will automatically pair

**Method 2: Using API**

```bash
curl -X POST http://192.168.4.1/api/setDevice \
  -H "Content-Type: application/json" \
  -d '{
    "address": "aa:a8:a2:15:78:d9",
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
  }'
```

**Method 3: Edit Config File**

1. Edit `data/config.json`:
   ```json
   {
       "addr": "aa:a8:a2:15:78:d9",
       "service": "0000feea-0000-1000-8000-00805f9b34fb",
       "charact": "00002aa1-0000-1000-8000-00805f9b34fb",
       ...
   }
   ```

2. Re-upload filesystem and restart

### 3.3 Verify Connection

Serial monitor should show:
```
FOUND MY DEVICE!
Connecting to aa:a8:a2:15:78:d9
CONNECTED TO DEVICE
CONNECTED TO SERVICE
SUBSCRIBED
```

## Step 4: Configure Barcode Mapping (2 minutes)

### 4.1 Create Shelf-to-Barcode Mapping

Edit `data/table.json` to map barcodes to shelf locations:

```json
{
    "ShelfA": ["1234567890", "0987654321"],
    "ShelfB": ["1111111111", "2222222222"],
    "ShelfC": ["3333333333"],
    "ShelfD": ["4444444444", "5555555555", "6666666666"]
}
```

**Format:**
- Key: Shelf/pin name (matches names in `config.json` pins array)
- Value: Array of barcodes that should light up this shelf

### 4.2 Configure Pin Names

Edit `data/config.json` to assign names to physical pins:

```json
{
    "pins": ["ShelfA", "ShelfB", "ShelfC", "ShelfD", ...],
    ...
}
```

**Pin array index = Physical LED pin number**
- `pins[0]` = Pin 0 (first LED)
- `pins[1]` = Pin 1 (second LED)
- etc.

### 4.3 Upload Updated Configuration

```bash
platformio run --target uploadfs
```

Or update via web interface (if supported).

## Step 5: Test the System (1 minute)

### 5.1 Test Individual LEDs

**Using Web Interface:**
1. Open device web page
2. Look for LED test buttons
3. Click pins to test each LED

**Using API:**
```bash
# Test pin 0
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 0}'

# Test all LEDs
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 48}'
```

### 5.2 Test Barcode Scanning

1. Scan a barcode that's mapped in `table.json`
2. Watch serial monitor for:
   ```
   R: 1234567890 = ShelfA
   Starting to blink 0
   ```
3. Corresponding LED should blink for 10 seconds

### 5.3 Monitor Real-Time Events

Open browser console and connect to event stream:

```javascript
const eventSource = new EventSource('http://192.168.4.1/events');

eventSource.addEventListener('scan', (event) => {
    const data = JSON.parse(event.data);
    console.log('Scanned:', data.code, 'Pin:', data.pin);
});

eventSource.addEventListener('status', (event) => {
    const data = JSON.parse(event.data);
    console.log('Status:', data);
});
```

## Common Quick Issues

### Device Won't Connect to WiFi

**Quick Fix:**
1. Set `"standalone": true` in `config.json`
2. Re-upload filesystem
3. Connect to device AP directly

### Scanner Not Pairing

**Quick Fix:**
1. Check scanner is in pairing mode (usually indicated by LED)
2. Verify correct MAC address (case-sensitive)
3. Power cycle both scanner and ESP32

### No LEDs Light Up

**Quick Fix:**
1. Test with pin 48 (all LEDs)
2. Check serial monitor for "Wire0/Wire1 not found!"
3. Verify CH423 I2C connections
4. Check LED polarity (try swapping if individual LED doesn't work)

### Barcode Doesn't Trigger LED

**Quick Fix:**
1. Check barcode exists in `table.json`
2. Verify pin name matches entry in `config.json` pins array
3. Monitor `/events` to see if scan is detected
4. Check serial output for "R: [code] = [pin]"

## Next Steps

Now that your system is running:

1. **Map all your shelves**: Update `table.json` with all product barcodes
2. **Label LEDs**: Physically label each LED with its pin number and shelf location
3. **Train users**: Show warehouse staff how to use the system
4. **Monitor performance**: Use the web interface to track scan events
5. **Optimize**: Adjust blink duration and patterns in code if needed

## Useful Commands Reference

### PlatformIO
```bash
# Build firmware
pio run

# Upload firmware
pio run --target upload

# Upload filesystem
pio run --target uploadfs

# Clean build
pio run --target clean

# Monitor serial
pio device monitor
```

### API Testing
```bash
# Get WiFi info
curl http://192.168.4.1/api/wifiInfo

# Blink specific LED
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 5}'

# Set scanner device
curl -X POST http://192.168.4.1/api/setDevice \
  -H "Content-Type: application/json" \
  -d '{"address": "aa:a8:a2:15:78:d9"}'
```

### Serial Monitor Shortcuts
- `Ctrl+C`: Exit monitor
- `Ctrl+T` then `Ctrl+D`: Toggle DTR
- `Ctrl+T` then `Ctrl+R`: Toggle RTS

## Support

For detailed information, see:
- [README.md](../README.md) - Complete project documentation
- [HARDWARE_SETUP.md](HARDWARE_SETUP.md) - Hardware assembly guide
- [API_REFERENCE.md](API_REFERENCE.md) - Complete API documentation
- [DEPLOYMENT.md](DEPLOYMENT.md) - Warehouse deployment guide
