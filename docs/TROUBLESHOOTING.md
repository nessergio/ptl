# Troubleshooting Guide

Comprehensive troubleshooting guide for the Pick-to-Light system.

## Table of Contents

- [Quick Diagnostics](#quick-diagnostics)
- [WiFi Connection Issues](#wifi-connection-issues)
- [Bluetooth Scanner Issues](#bluetooth-scanner-issues)
- [LED Problems](#led-problems)
- [Barcode Mapping Issues](#barcode-mapping-issues)
- [Performance Issues](#performance-issues)
- [Hardware Failures](#hardware-failures)
- [Firmware Issues](#firmware-issues)
- [Serial Monitor Error Messages](#serial-monitor-error-messages)

---

## Quick Diagnostics

### Health Check Procedure

Run through this checklist when troubleshooting:

1. **Power Check**
   - [ ] Device powered on (check LEDs on ESP32 board)
   - [ ] Power supply voltage: 5V ± 0.25V
   - [ ] Sufficient current capacity (10A recommended)

2. **Network Check**
   - [ ] WiFi connected (check serial monitor)
   - [ ] Device IP accessible (ping test)
   - [ ] Web interface loads

3. **Scanner Check**
   - [ ] Scanner powered on
   - [ ] Scanner battery charged
   - [ ] Scanner connected (check status via `/events` endpoint)

4. **LED Check**
   - [ ] Test all LEDs using pin 48
   - [ ] Individual LED test via web interface
   - [ ] Check for burned-out LEDs

5. **Configuration Check**
   - [ ] `config.json` valid JSON
   - [ ] `table.json` valid JSON
   - [ ] Pin names match between files

### Serial Monitor Commands

Connect to device serial monitor (115200 baud) to see:

```bash
platformio device monitor --baud 115200
```

**Expected output on boot:**
```
Running ble on core 0
Running blink on core 0
Init BLE ok
SPIFFS mounted successfully
Config opened!
Table opened!
Connecting to WiFi (WarehouseWiFi)
192.168.1.100
```

### Quick Test API Calls

```bash
# Test connectivity
curl http://192.168.4.1/api/wifiInfo

# Test LED 0
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 0}'

# Test all LEDs
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 48}'
```

---

## WiFi Connection Issues

### Device Won't Connect to WiFi

**Symptoms:**
- Serial monitor shows continuous dots: `.......`
- Never displays IP address
- Can't access web interface

**Diagnosis:**

Check serial output:
```
Connecting to WiFi (YourSSID)
........................................
```

**Common Causes & Solutions:**

#### 1. Wrong SSID or Password

**Check:**
```json
// In config.json
{
    "ssid": "YourNetworkName",  // Case-sensitive!
    "wifipass": "YourPassword"   // Check for typos
}
```

**Fix:**
- Verify SSID matches exactly (including spaces, case)
- Confirm password
- Re-upload filesystem: `pio run --target uploadfs`
- Reboot device

#### 2. Network Uses 5GHz Only

**Problem:** ESP32 only supports 2.4GHz WiFi

**Fix:**
- Enable 2.4GHz on your router
- Or create separate 2.4GHz network
- Update `config.json` with 2.4GHz network name

#### 3. WPA3 or Enterprise Security

**Problem:** ESP32 supports WPA/WPA2-PSK only

**Fix:**
- Set router to WPA2-PSK mode
- Use different network with compatible security
- Update router settings

#### 4. MAC Address Filtering

**Problem:** Router blocks unknown MAC addresses

**Check:** Router's MAC filter list

**Fix:**
- Find ESP32 MAC address in serial output
- Add to router's allowed devices list
- Or disable MAC filtering

**Get MAC address:**
```cpp
// Add to setup() in main.cpp temporarily
Serial.print("MAC: ");
Serial.println(WiFi.macAddress());
```

### Device Creates AP Instead of Connecting

**Symptoms:**
- Device creates WiFi network instead of connecting
- Shows `192.168.4.1` as IP

**Cause:** `standalone` set to `true` in config

**Fix:**
```json
{
    "standalone": false,  // Change from true
    "ssid": "YourNetwork",
    "wifipass": "YourPassword"
}
```

Upload and reboot.

### IP Address Conflict

**Symptoms:**
- Device connects but can't be reached
- Another device has same IP
- Intermittent connectivity

**Check:**
```bash
# From computer on same network
ping 192.168.1.100

# Check for duplicate
arp -a | grep 192.168.1.100
```

**Fix:**
```json
// Change to unused IP
{
    "cidr": "192.168.1.101/24"  // Different IP
}
```

### Weak WiFi Signal

**Symptoms:**
- Frequent disconnections
- Slow response
- Serial shows reconnection attempts

**Diagnosis:**
```
WiFi signal strength (RSSI): -75 dBm  // Weak!
```

**Signal Quality:**
| RSSI | Quality |
|------|---------|
| -30 to -50 dBm | Excellent |
| -50 to -60 dBm | Good |
| -60 to -70 dBm | Fair |
| -70 to -80 dBm | Poor |
| -80+ dBm | Very Poor |

**Solutions:**
1. Move device closer to access point
2. Add WiFi repeater/extender
3. Use directional antenna on ESP32
4. Switch to wired Ethernet (requires adapter)
5. Reduce obstacles between device and AP

---

## Bluetooth Scanner Issues

### Scanner Won't Pair

**Symptoms:**
- Serial shows `Scanning BLE...` but never connects
- Status shows `STATUS_DEVICE_NOT_CONNECTED`

**Diagnosis Steps:**

1. **Check if scanner is visible:**

Monitor serial output during scan:
```
Scanning BLE...
A: aa:a8:a2:15:78:d9 N: Scanner S: 0000feea-...
```

If scanner doesn't appear:
- Scanner not in pairing mode
- Scanner already paired to another device
- Scanner out of range
- Scanner battery dead

2. **Check configuration:**

```json
{
    "addr": "aa:a8:a2:15:78:d9",  // Must match scanner exactly
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

**Common Fixes:**

#### Scanner Not Advertising

**Fix:**
- Power cycle scanner
- Check scanner manual for pairing mode
- Ensure scanner is unpaired from other devices
- Replace scanner batteries

#### Wrong MAC Address

**Serial shows:**
```
A: bb:b8:b2:25:88:e9  // Different from config
```

**Fix:**
```json
// Update to actual address
{
    "addr": "bb:b8:b2:25:88:e9"
}
```

#### Case Sensitivity

**Wrong:**
```json
"addr": "AA:A8:A2:15:78:D9"  // Uppercase
```

**Correct:**
```json
"addr": "aa:a8:a2:15:78:d9"  // Lowercase
```

### Scanner Connects But No Scans Received

**Symptoms:**
- Serial shows `CONNECTED TO DEVICE` and `SUBSCRIBED`
- Scanning doesn't trigger LEDs
- No scan events in serial monitor

**Check Serial Output:**
```
CONNECTED TO DEVICE
CONNECTED TO SERVICE
SUBSCRIBED  ← Should see this
```

If `SUBSCRIBED` missing:

**Problem:** Wrong characteristic UUID

**Fix:**
1. Use BLE scanner app (nRF Connect, LightBlue)
2. Find characteristic with "Notify" property
3. Update `config.json`:
```json
{
    "charact": "correct-uuid-here"
}
```

**Or try auto-detection:**
```json
{
    "charact": ""  // Leave empty to auto-detect
}
```

### Scanner Disconnects Frequently

**Symptoms:**
- Scanner connects then disconnects
- Serial alternates between connected/disconnected
- Scans work intermittently

**Causes & Solutions:**

#### 1. Low Battery

**Check:** Scanner battery level (usually LED indicator)

**Fix:** Replace or recharge batteries

#### 2. Out of Range

**Problem:** Scanner too far from ESP32

**BLE Range:**
- Typical: 10 meters (33 feet)
- Obstacles reduce range significantly

**Fix:**
- Move closer to device
- Remove obstacles
- Use scanner with external antenna
- Use ESP32 with external antenna

#### 3. Interference

**Sources:**
- WiFi (operates on 2.4GHz like BLE)
- Microwave ovens
- Other BLE devices
- USB 3.0 cables

**Fix:**
- Reduce WiFi transmit power
- Move away from interference sources
- Use different BLE channel (requires scanner config)

#### 4. Scanner Sleep Mode

**Problem:** Scanner sleeps after inactivity

**Fix:**
- Configure scanner for no sleep/always-on mode
- Or accept reconnection delay

### Multiple Scanners Detected

**Symptom:** Serial shows many devices

**Problem:** Many BLE devices in area

**Fix:**
Filter by specific MAC address in config (already implemented).

System only connects to device matching `addr` in config.

### Wrong Barcodes Received

**Symptoms:**
- Scan events show incorrect data
- Extra characters in barcode

**Diagnosis:**

Monitor serial when scanning barcode `123456`:
```
R: PREFIX123456SUFFIX = ShelfA
```

**Causes:**
1. Scanner configured with prefix/suffix
2. Scanner in wrong mode (e.g., sending keyboard codes)
3. Character encoding issues

**Fix:**
1. Configure scanner to send raw data only
2. Or update `table.json` to include prefix/suffix:
```json
{
    "ShelfA": ["PREFIX123456SUFFIX"]
}
```

---

## LED Problems

### No LEDs Work

**Symptoms:**
- None of the LEDs light up
- Test with pin 48 fails
- All manual tests fail

**Serial Diagnostics:**

Check for I2C errors:
```
Wire0 not found!  ← CH423 #1 not detected
Wire1 not found!  ← CH423 #2 not detected
```

**Causes & Solutions:**

#### 1. CH423 Not Connected

**Check:**
- I2C wiring (SDA, SCL, VCC, GND)
- Connections secure
- No shorts

**I2C Wiring:**
```
ESP32          CH423 #1
GPIO21 (SDA) ─────> SDA
GPIO22 (SCL) ─────> SCL
3.3V         ─────> VCC
GND          ─────> GND
```

#### 2. I2C Pull-up Resistors Missing

**Add:** 4.7kΩ resistors from SDA and SCL to 3.3V

#### 3. Power Issue

**Check:**
- CH423 VCC voltage (should be 3.3V)
- Power supply capacity
- Voltage drops under load

**Measure:**
```
Multimeter between CH423 VCC and GND: 3.3V ± 0.15V
```

#### 4. Wrong I2C Address

**Test with I2C scanner:**

Add to setup():
```cpp
Wire.begin();
for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
        Serial.printf("Device at 0x%02X\n", addr);
    }
}
```

Should find CH423 at `0x24` (when shifted).

### Some LEDs Don't Work

**Symptoms:**
- Specific LEDs never illuminate
- Other LEDs work fine
- Problem consistent (same LEDs)

**Diagnosis:**

Test each LED:
```bash
for i in {0..47}; do
    curl -X POST http://192.168.4.1/api/blink \
      -H "Content-Type: application/json" \
      -d "{\"pin\": $i}"
    sleep 2
done
```

**Causes & Solutions:**

#### 1. Burned Out LED

**Check:** Measure voltage across LED when activated

**Fix:** Replace LED

#### 2. Wiring Issue

**Check:**
- LED polarity correct
- Solder joints
- Cable continuity

**Test:**
```
Multimeter continuity test:
CH423 pin → LED anode → LED cathode → GND
```

#### 3. Wrong Pin Mapping

**Problem:** Physical LED connected to different pin than expected

**Fix:**
- Document actual physical pin connections
- Update code/mapping if needed
- Re-label LEDs

### LEDs Flicker or Dim

**Symptoms:**
- LEDs flicker randomly
- Brightness varies
- Unstable illumination

**Causes & Solutions:**

#### 1. Insufficient Power

**Check:**
- Measure voltage under load
- Calculate total current draw

**Current calculation:**
```
N LEDs × 20mA = Total current
48 LEDs × 20mA = 960mA

Add ESP32: ~250mA
Total: ~1.2A minimum
```

**Fix:**
- Use larger power supply
- Reduce LED current (increase resistor values)
- Power LEDs from separate supply

#### 2. Voltage Drop in Wiring

**Problem:** Long wires, thin gauge

**Fix:**
- Use thicker wire (18-20 AWG for power)
- Shorter cable runs
- Multiple power injection points

#### 3. Ground Loops

**Problem:** Poor ground connections

**Fix:**
- Single common ground point
- Star ground configuration
- Thick ground wires

### NeoPixel Strip Not Working

**Symptoms:**
- WS2812B strip shows no colors
- Or shows random colors
- Main LED indicators work fine

**Checks:**

1. **Power:**
   - 5V supply to strip
   - Ground connected
   - Sufficient current (60mA per LED max)

2. **Data:**
   - GPIO16 connected to DIN
   - 330-470Ω resistor in series
   - Signal level converter if needed (3.3V → 5V)

3. **Code:**
   - Check `main.cpp` lines 322-329 for NeoPixel code
   - Verify `numOfLeds` setting

**Fix:**
```cpp
// In main.cpp, adjust if needed
const int numOfLeds = 60;  // Match your strip
```

---

## Barcode Mapping Issues

### Barcode Doesn't Trigger LED

**Symptoms:**
- Scan detected (shows in serial/events)
- No LED lights up
- Pin field empty in scan event

**Serial Output:**
```
R: 1234567890 =   ← Empty after equals sign
```

**Diagnosis:**

1. **Check barcode in table.json:**
```bash
grep "1234567890" data/table.json
```

If not found, barcode not mapped.

2. **Check exact string:**

Monitor serial for actual scanned value:
```
R: CODE1234567890 =   ← Note prefix "CODE"
```

**Fixes:**

#### Barcode Not in Mapping

Add to `table.json`:
```json
{
    "ShelfA": ["1234567890"]
}
```

Upload and restart.

#### Extra Characters

Scanner adding prefix/suffix:
```json
// Match what scanner actually sends
{
    "ShelfA": ["CODE1234567890"]
}
```

#### Case Sensitivity

Barcode comparison is case-sensitive:
```json
// If scanner sends lowercase
{
    "ShelfA": ["abc123"]  // Not "ABC123"
}
```

### Wrong LED Lights Up

**Symptoms:**
- Scan works but lights wrong shelf
- Consistent problem with specific barcode

**Diagnosis:**

Check mapping in `table.json`:
```json
{
    "ShelfA": ["123456"],  // Scan lights up pin for ShelfA
    "ShelfB": ["789012"]
}
```

And `config.json`:
```json
{
    "pins": ["ShelfA", "ShelfB", ...]
                ↑           ↑
            Pin 0       Pin 1
}
```

**Causes:**

1. **Mapping is correct, physical labels wrong:**
   - LED labeled "ShelfA" actually connected to wrong pin
   - Fix: Re-label LEDs or rewire

2. **Duplicate barcode:**
   ```json
   {
       "ShelfA": ["123", "456"],
       "ShelfB": ["789", "123"]  // Duplicate!
   }
   ```
   System uses first match found.

3. **Pin name mismatch:**
   ```json
   // config.json
   "pins": ["ShelfA", "ShelfB"]

   // table.json
   {
       "Shelf-A": ["123"]  // Doesn't match "ShelfA"
   }
   ```

### Mapping Changes Don't Take Effect

**Symptoms:**
- Updated `table.json`
- Re-uploaded filesystem
- Still uses old mapping

**Cause:** Config cached, not reloaded

**Fix:**
```bash
# Full procedure
1. Edit data/table.json
2. Upload filesystem: pio run --target uploadfs
3. Press ESP32 reset button (or power cycle)
4. Monitor serial to confirm: "Table opened!"
```

Or use API (if implemented):
```bash
curl -X POST http://192.168.4.1/api/reloadTable
```

### JSON Syntax Errors

**Symptoms:**
- Serial shows `ERROR: deserialize`
- Mapping doesn't load
- System uses old/empty mapping

**Validate JSON:**
```bash
python3 -m json.tool data/table.json
```

**Common errors:**

```json
// Missing comma
{
    "ShelfA": ["123"]  ← Missing comma
    "ShelfB": ["456"]
}

// Extra comma
{
    "ShelfA": ["123"],
    "ShelfB": ["456"],  ← Remove
}

// Single quotes (use double)
{
    'ShelfA': ['123']  ← Use "ShelfA": ["123"]
}

// Missing quotes
{
    ShelfA: ["123"]  ← Use "ShelfA"
}
```

---

## Performance Issues

### Slow Response Time

**Symptoms:**
- Long delay between scan and LED
- Laggy web interface
- API calls slow

**Measure latency:**
```bash
time curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 0}'
```

Should be <1 second.

**Causes & Solutions:**

#### 1. WiFi Issues

**Check:** RSSI signal strength (see WiFi section)

**Fix:**
- Improve WiFi signal
- Reduce network congestion
- Use wired connection

#### 2. Memory Issues

**Serial shows:**
```
HEAP: 45000  ← Low free heap!
```

**Normal:** 80,000-150,000 bytes free

**Fix:**
- Simplify `table.json` (fewer entries)
- Reduce other memory usage
- Check for memory leaks (constant decrease)

#### 3. CPU Load

**Cause:** NeoPixel animation in main loop

**Fix:** Reduce or disable:
```cpp
// In main.cpp loop(), comment out:
/*
for(int x=0; x<numOfLeds; x++){
    // NeoPixel animation
}
strip.show();
*/
```

### System Freezes or Reboots

**Symptoms:**
- Device stops responding
- Automatic restarts
- Watchdog timer resets

**Serial shows:**
```
Guru Meditation Error: Core 0 panic'ed (LoadProhibited)
```

**Causes:**

1. **Memory corruption**
2. **Stack overflow**
3. **Power issues (brownout)**
4. **Hardware fault**

**Diagnosis:**

Enable exception decoder:
```ini
# platformio.ini already has:
monitor_filters = esp32_exception_decoder
```

**Fixes:**

1. **Check power supply:**
   - Minimum 5V 2A
   - Measure voltage during operation
   - Add bulk capacitor (1000µF) near ESP32

2. **Reduce stack usage:**
   - Smaller buffers
   - Simpler JSON documents

3. **Update firmware:**
   - Latest PlatformIO platform
   - Update libraries

---

## Hardware Failures

### ESP32 Won't Boot

**Symptoms:**
- No serial output
- LEDs on board don't light
- USB not recognized

**Checks:**

1. **Power:** Measure 3.3V and 5V rails
2. **USB cable:** Try different cable (data, not charge-only)
3. **USB port:** Try different computer port
4. **Boot mode:** GPIO0 not stuck low

**Fix:**
- Replace ESP32 board
- Check for shorts on PCB
- Try entering flash mode manually (hold BOOT, press RESET)

### CH423 Failures

**Symptoms:**
- `Wire not found!` errors
- Intermittent I2C communication
- Some but not all LEDs work

**Test:**
```cpp
Wire.beginTransmission(0x24);
uint8_t error = Wire.endTransmission();
// error == 0: Success
// error == 2: NACK on address
// error == 4: Other error
```

**Fix:**
- Check I2C wiring
- Replace CH423 chip
- Verify I2C clock speed (400kHz)

### Power Supply Failures

**Symptoms:**
- Brownouts
- Random restarts
- LEDs dim when many active

**Measure:**
```
Voltage under max load (all LEDs on): 5V ± 5%
Ripple: <100mV peak-to-peak
```

**Fix:**
- Replace with higher-capacity supply
- Add local filtering (capacitors)
- Separate supplies for ESP32 and LEDs

---

## Firmware Issues

### Upload Fails

**Error:**
```
Failed to connect to ESP32: Wrong boot mode detected
```

**Fix:**
1. Hold BOOT button on ESP32
2. Press and release RESET
3. Release BOOT
4. Try upload again

Or:
```bash
pio run --target upload --upload-port /dev/ttyUSB0
```

### Filesystem Upload Fails

**Error:**
```
SPIFFS upload failed
```

**Fix:**
```bash
# Erase flash first
pio run --target erase

# Then upload
pio run --target upload
pio run --target uploadfs
```

### Firmware Corrupted

**Symptoms:**
- Erratic behavior
- Features don't work
- Different from expected

**Fix:** Re-flash complete firmware:
```bash
pio run --target erase
pio run --target upload
pio run --target uploadfs
```

---

## Serial Monitor Error Messages

### Common Errors and Meanings

| Error Message | Meaning | Fix |
|--------------|---------|-----|
| `An error has occurred while mounting SPIFFS` | Filesystem not formatted | Run `uploadfs` |
| `ERROR: There was an error opening config file` | config.json missing | Upload data folder |
| `ERROR: deserialize` | Invalid JSON syntax | Validate JSON files |
| `Wire0 not found!` | CH423 #1 not detected | Check I2C wiring |
| `Wire1 not found!` | CH423 #2 not detected | Normal if only using one |
| `ERROR CONNECTING TO DEVICE` | BLE connection failed | Check scanner address |
| `ERROR CONNECTING TO SERVICE` | Wrong service UUID | Verify in config |
| `NO VALUABLE CHARACTERISTIC FOUND` | Can't find notify characteristic | Update charact in config |
| `Guru Meditation Error` | Crash/exception | Check stack trace |

---

## Getting Help

### Information to Collect

When reporting issues:

1. **Serial monitor output** (full boot sequence)
2. **Configuration files** (config.json, table.json)
3. **Hardware details** (ESP32 model, CH423 count, LED count)
4. **Network environment** (WiFi vs AP, router model)
5. **Scanner model** and specifications
6. **Steps to reproduce** the problem
7. **Expected vs actual behavior**

### Debug Mode

Enable verbose logging (requires code modification):

```cpp
// In main.cpp, add:
#define DEBUG 1

#if DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Then use throughout code:
DEBUG_PRINTLN("Detailed debug message");
```

---

## Preventive Maintenance

### Weekly Checks
- [ ] Test all LEDs
- [ ] Check scanner battery
- [ ] Verify WiFi connection
- [ ] Review error logs

### Monthly Checks
- [ ] Clean dust from enclosure
- [ ] Inspect cables for wear
- [ ] Update firmware if available
- [ ] Verify barcode mappings accurate

### Annual Checks
- [ ] Replace aging LEDs
- [ ] Update all firmware
- [ ] Review and optimize configuration
- [ ] Test backup/restore procedures

---

**Document Version:** 1.0
**Last Updated:** 2024
