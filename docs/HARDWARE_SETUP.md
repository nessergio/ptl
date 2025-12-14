# Hardware Setup Guide

This guide provides detailed instructions for assembling and wiring the Pick-to-Light hardware system.

## Bill of Materials

### Required Components

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32 Development Board | 1 | ESP32-DOIT-DevKit-V1 or compatible |
| CH423 I/O Expander | 1-2 | Up to 2 for 48 LED outputs |
| RGB LED indicators | 1-48 | Common anode or cathode depending on design |
| LED driver circuits | 1-48 | Transistors or MOSFETs if needed |
| WS2812B LED Strip (optional) | 1 | 60 LEDs for additional visual feedback |
| Bluetooth Barcode Scanner | 1 | BLE-enabled scanner |
| Power Supply | 1 | 5V for ESP32 and LEDs (current depends on LED count) |
| Resistors | Various | Pull-ups, current limiting |
| PCB or Breadboard | 1 | For prototyping or permanent installation |

### Optional Components
- Level shifters (if using 5V LEDs with 3.3V ESP32 logic)
- Enclosure for electronics
- Mounting hardware for shelf installation
- Ethernet adapter (for PoE installations)

## Wiring Diagram

### ESP32 to CH423 I/O Expanders

#### First CH423 (Pins 0-23)
```
ESP32          CH423 #1
GPIO21 (SDA) ─────> SDA
GPIO22 (SCL) ─────> SCL
3.3V         ─────> VCC
GND          ─────> GND
```

#### Second CH423 (Pins 24-47) - Optional
```
ESP32          CH423 #2
GPIO18 (SDA) ─────> SDA
GPIO19 (SCL) ─────> SCL
3.3V         ─────> VCC
GND          ─────> GND
```

### NeoPixel LED Strip (Optional)
```
ESP32          WS2812B Strip
GPIO16       ─────> DIN (Data In)
5V           ─────> VCC
GND          ─────> GND
```

**Note**: Add a 330-470Ω resistor between GPIO16 and DIN, and a 1000µF capacitor between VCC and GND on the LED strip.

### CH423 to LED Indicators

Each CH423 provides:
- 8 GPIO pins (bidirectional, open-drain)
- 16 GPO pins (output-only, push-pull)

#### For Common Cathode RGB LEDs
```
CH423 Pin    LED Connection
GPO0-15   ──> LED Anode (through current limiting resistor)
GND       ──> LED Cathode
```

#### For Common Anode RGB LEDs
```
CH423 Pin    LED Connection
GPO0-15   ──> LED Cathode (through transistor base)
VCC       ──> LED Anode
```

## Power Calculations

### LED Current Requirements
- Single-color LED: ~20mA @ full brightness
- RGB LED module: ~60mA @ full white (all colors on)

### Total System Power
```
Component                Current Draw
ESP32                    ~250mA (peak during WiFi/BT)
CH423 #1                 ~10mA
CH423 #2                 ~10mA
48 LEDs @ 20mA each      ~960mA (maximum)
NeoPixel strip (60 LEDs) ~3.6A @ full white (60mA × 60)
----------------------------------------
Minimum 5V supply        ~5A (conservative)
```

**Recommendation**: Use a 5V 10A power supply for systems with NeoPixel strips and all LEDs potentially on simultaneously.

## Assembly Instructions

### Step 1: Prepare the PCB or Breadboard

1. **Using Custom PCB**: Use the provided Gerber files in `/pcb/` directory
   - Send Gerber files to PCB manufacturer
   - Order components from BOM files
   - Solder components according to pick-and-place files

2. **Using Breadboard** (prototyping):
   - Use a large breadboard or multiple breadboards
   - Plan layout to minimize wire length
   - Keep I2C lines short and away from noise sources

### Step 2: Install CH423 I/O Expanders

1. Mount CH423 modules on PCB or breadboard
2. Connect I2C lines:
   - CH423 #1: Default I2C bus (GPIO21/22)
   - CH423 #2: Secondary I2C bus (GPIO18/19)
3. Add pull-up resistors if not already on modules:
   - 4.7kΩ from SDA to 3.3V
   - 4.7kΩ from SCL to 3.3V

### Step 3: Connect LED Indicators

1. **For each LED position**:
   ```
   CH423 Pin → Current Limiting Resistor (220-330Ω) → LED Anode
   LED Cathode → GND
   ```

2. **Calculate resistor value**:
   ```
   R = (Vsupply - VLED) / ILED

   Example: 5V supply, 2V red LED, 20mA current
   R = (5V - 2V) / 0.02A = 150Ω
   Use nearest standard value: 220Ω
   ```

3. **Label each LED** with its pin number (0-47) for easy troubleshooting

### Step 4: Mount ESP32

1. Install ESP32 development board
2. Connect power rails (3.3V, 5V, GND)
3. Connect I2C lines to CH423 chips
4. Connect GPIO16 to NeoPixel strip (if using)

### Step 5: Power Supply Connection

1. Connect 5V power supply to:
   - ESP32 VIN pin (if accepting 5V input)
   - CH423 VCC pins
   - LED power rails
   - NeoPixel strip VCC

2. Connect all grounds together (common ground)

3. **Important**: Use thick wires (18-22 AWG) for power distribution

### Step 6: Strain Relief and Mechanical

1. Secure all connections with heat shrink or electrical tape
2. Add strain relief to power cables
3. Mount PCB in enclosure if using
4. Ensure proper ventilation for ESP32 and power supply

## Shelf Installation

### LED Indicator Placement

1. **Determine shelf mapping**:
   - Assign each shelf/bin a unique pin number (0-47)
   - Document the physical location to pin mapping
   - Update `config.json` with pin names/labels

2. **Mount LED indicators**:
   - Position LEDs for maximum visibility
   - Use bright, high-intensity LEDs for large warehouses
   - Angle LEDs toward worker sight lines
   - Consider LED color: Red, Green, or RGB for different priority levels

3. **Cable routing**:
   - Run cables from control box to each shelf
   - Use cable ties or conduit for neat installation
   - Label cables at both ends with pin numbers
   - Leave service loops for future adjustments

### Control Box Placement

- Mount near power outlet
- Position for easy WiFi/network access
- Allow ventilation around enclosure
- Consider cable run lengths to shelves (max recommended: 10m)

## Testing Procedure

### Pre-Power Tests

1. **Visual inspection**: Check for:
   - Proper wire connections
   - No shorts between power and ground
   - Correct LED polarity
   - Secure component mounting

2. **Continuity test**: Verify:
   - Ground continuity across all components
   - I2C connections between ESP32 and CH423
   - No shorts on power rails

### Initial Power-Up

1. **Power on with limited current**:
   - Use a current-limited power supply (500mA limit initially)
   - Watch for smoke, unusual heat, or burning smell
   - Check voltage levels: 5V and 3.3V rails

2. **Check serial output** (115200 baud):
   ```
   Expected messages:
   - "Running ble on core 0"
   - "Running blink on core 0"
   - "Init BLE ok"
   - "Wire0 not found!" or CH423 initialization success
   - "Wire1 not found!" or CH423 initialization success
   ```

### LED Testing

1. **Test all LEDs** using web interface:
   - Navigate to device IP address
   - Use manual LED test feature
   - Click each pin to verify LED illuminates

2. **Test using API**:
   ```bash
   # Test pin 0
   curl -X POST http://[device-ip]/api/blink \
     -H "Content-Type: application/json" \
     -d '{"pin": 0}'

   # Test all LEDs
   curl -X POST http://[device-ip]/api/blink \
     -H "Content-Type: application/json" \
     -d '{"pin": 48}'
   ```

3. **Verify NeoPixel strip**:
   - Should display random colors on startup
   - Indicates successful initialization

### BLE Scanner Testing

1. **Check scanner is discoverable**:
   - Power on Bluetooth scanner
   - Look for "Scanning BLE..." in serial monitor
   - Verify scanner MAC address appears in logs

2. **Configure scanner pairing**:
   - Note scanner BLE address from serial output
   - Update `config.json` with correct address
   - Restart device

3. **Test scanning**:
   - Scan a barcode with paired scanner
   - Serial monitor should show: "R: [barcode] = [pin]"
   - Corresponding LED should blink

## Troubleshooting

### No LEDs Work

**Check:**
- Power supply voltage and current capacity
- CH423 I2C addresses responding (check serial monitor)
- Ground connections
- LED polarity (swap if needed)

**Test:**
- Measure voltage at LED pins with multimeter
- Try pin 48 to activate all LEDs

### Some LEDs Don't Work

**Check:**
- Individual LED connections
- Resistor values
- Burned out LEDs (measure voltage across LED)

**Fix:**
- Replace faulty LEDs
- Check for cold solder joints

### CH423 Not Detected

**Check:**
- I2C wiring (SDA, SCL, VCC, GND)
- Pull-up resistors on I2C lines
- CH423 power supply

**Test:**
- Use I2C scanner sketch to detect devices
- Measure voltage on CH423 VCC pin (should be 3.3V)

### High Current Draw

**Check:**
- LED current limiting resistors
- Multiple LEDs illuminated simultaneously
- NeoPixel strip brightness settings

**Fix:**
- Increase resistor values to reduce LED current
- Reduce NeoPixel brightness in code
- Use larger power supply

## Safety Considerations

1. **Electrical Safety**:
   - Ensure all connections are insulated
   - Use properly rated power supply
   - Add fuses or circuit breakers
   - Keep electronics away from moisture

2. **Fire Prevention**:
   - Don't exceed component current ratings
   - Ensure adequate ventilation
   - Use UL-listed power supplies
   - Monitor for overheating during initial operation

3. **Mechanical Safety**:
   - Secure all mounting hardware
   - Prevent trip hazards with cable management
   - Protect exposed electronics in industrial environments

## Maintenance

### Regular Checks (Monthly)
- Inspect cable connections
- Clean dust from enclosure vents
- Test all LED indicators
- Verify BLE scanner battery level

### Annual Maintenance
- Replace any failed LEDs
- Check power supply output voltage
- Tighten all screw terminals
- Update firmware if available

## Upgrades and Modifications

### Adding More LEDs
- Chain additional CH423 modules (requires code modification)
- Use I2C multiplexer for more than 2 CH423 chips

### Improving Range
- Use WiFi extender or mesh network
- Add external BLE antenna to ESP32
- Use PoE (Power over Ethernet) adapter

### Environmental Protection
- Use IP-rated enclosures for dusty/wet environments
- Apply conformal coating to PCBs
- Use industrial-grade components for extreme temperatures
