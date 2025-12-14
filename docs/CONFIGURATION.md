# Configuration Reference

Complete reference for configuring the Pick-to-Light system.

## Configuration Files

The system uses two main JSON configuration files stored in the SPIFFS filesystem:

1. **config.json** - Device settings, WiFi, BLE scanner
2. **table.json** - Barcode to shelf location mapping

## config.json Reference

### Location

`data/config.json` (uploaded to SPIFFS filesystem)

### Complete Example

```json
{
    "standalone": false,
    "ssid": "WarehouseWiFi",
    "wifipass": "SecurePassword123",
    "cidr": "192.168.88.201/24",
    "gw": "192.168.88.1",
    "addr": "aa:a8:a2:15:78:d9",
    "pins": [
        "ShelfA1", "ShelfA2", "ShelfA3", "ShelfA4",
        "ShelfB1", "ShelfB2", "ShelfB3", "ShelfB4",
        "ShelfC1", "ShelfC2", "ShelfC3", "ShelfC4"
    ],
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

### Field Reference

#### `standalone` (boolean)

**Description:** WiFi operation mode

**Values:**
- `true` - Access Point mode (device creates its own WiFi network)
- `false` - Station mode (device connects to existing WiFi network)

**Default:** `false`

**Examples:**
```json
// For portable installations or no WiFi available
"standalone": true

// For permanent installation with existing WiFi
"standalone": false
```

**Notes:**
- In AP mode, device IP is `192.168.4.1`
- In Station mode, IP is assigned by DHCP or set via `cidr`
- Changing this requires device reboot

---

#### `ssid` (string)

**Description:** WiFi network name

**Format:** String, max 32 characters

**Usage:**
- **Station mode:** SSID of the network to connect to
- **AP mode:** Name of the access point to create

**Examples:**
```json
// Station mode - connect to existing network
"ssid": "WarehouseWiFi"

// AP mode - create network with this name
"ssid": "PTL-Device-01"
```

**Best Practices:**
- Use descriptive names for AP mode: `PTL-Zone-A`
- For station mode, match your network's SSID exactly (case-sensitive)

---

#### `wifipass` (string)

**Description:** WiFi password

**Format:** String, 8-63 characters for WPA/WPA2

**Security:**
- Required for station mode
- Required for AP mode (recommended for security)
- Stored in plain text - secure device access

**Examples:**
```json
"wifipass": "MySecurePassword123"
```

**Best Practices:**
- Use strong passwords (12+ characters)
- Include letters, numbers, and symbols
- Change default passwords before deployment
- Document passwords securely

---

#### `cidr` (string)

**Description:** Static IP address in CIDR notation (Station mode only)

**Format:** `IP_ADDRESS/SUBNET_BITS`

**Examples:**
```json
// Device IP: 192.168.1.100, Subnet: 255.255.255.0
"cidr": "192.168.1.100/24"

// Device IP: 10.0.50.25, Subnet: 255.255.0.0
"cidr": "10.0.50.25/16"
```

**Common Subnet Masks:**
| CIDR | Subnet Mask | Hosts |
|------|-------------|-------|
| /24 | 255.255.255.0 | 254 |
| /16 | 255.255.0.0 | 65,534 |
| /8 | 255.0.0.0 | 16,777,214 |

**Notes:**
- If omitted, DHCP is used
- Static IP recommended for production deployments
- Ensure IP doesn't conflict with DHCP range

---

#### `gw` (string)

**Description:** Gateway IP address (Station mode only)

**Format:** IPv4 address

**Examples:**
```json
"gw": "192.168.1.1"
```

**Notes:**
- Usually your router's IP address
- Required when using static IP (`cidr`)
- Verify with `ipconfig` (Windows) or `ip route` (Linux)

---

#### `addr` (string)

**Description:** Bluetooth MAC address of barcode scanner

**Format:** Lowercase hex bytes separated by colons

**Examples:**
```json
// Correct format
"addr": "aa:a8:a2:15:78:d9"

// Wrong - uppercase (may not work)
"addr": "AA:A8:A2:15:78:D9"

// Wrong - dashes instead of colons
"addr": "aa-a8-a2-15-78-d9"
```

**How to Find:**

1. Check serial monitor when scanner is advertising:
   ```
   Scanning BLE...
   A: aa:a8:a2:15:78:d9 N: Scanner S: 0000feea...
   ```

2. Or use smartphone BLE scanner app

**Notes:**
- Case-sensitive (use lowercase)
- Must match scanner exactly
- Leave empty (`""`) if scanner not yet known
- Device will scan and display available scanners

---

#### `pins` (array of strings)

**Description:** Names/labels for each physical LED pin

**Format:** Array of strings, up to 48 elements

**Index Mapping:**
- `pins[0]` = Physical pin 0
- `pins[1]` = Physical pin 1
- ...
- `pins[47]` = Physical pin 47

**Examples:**

```json
// Simple alphanumeric labels
"pins": ["A", "B", "C", "D", "E", "F"]

// Descriptive shelf locations
"pins": [
    "ShelfA-Top",
    "ShelfA-Middle",
    "ShelfA-Bottom",
    "ShelfB-Top"
]

// Warehouse grid notation
"pins": [
    "A-01-01",  // Aisle A, Bay 01, Level 01
    "A-01-02",
    "A-01-03",
    "A-02-01",
    "B-01-01"
]

// Route-based sorting
"pins": [
    "Route-101",
    "Route-102",
    "Route-103",
    "Local-Pickup",
    "Will-Call"
]
```

**Best Practices:**

1. **Use consistent naming conventions:**
   ```json
   // Good - consistent format
   "pins": ["A-01", "A-02", "A-03", "B-01"]

   // Bad - inconsistent
   "pins": ["ShelfA", "B-2", "C_3", "Location4"]
   ```

2. **Make names meaningful:**
   ```json
   // Good - descriptive
   "pins": ["Electronics-Bin-1", "Electronics-Bin-2"]

   // Bad - generic
   "pins": ["Bin1", "Bin2"]
   ```

3. **Scale for growth:**
   ```json
   // Good - room to grow
   "pins": ["A-001", "A-002", "A-003"]  // Can go to A-999

   // Bad - limited
   "pins": ["A-1", "A-2", "A-3"]  // Ambiguous after A-9
   ```

4. **Document mapping:**
   Create a spreadsheet mapping pin indices to physical locations:
   | Pin Index | Pin Name | Physical Location | Notes |
   |-----------|----------|-------------------|-------|
   | 0 | A-01-01 | Aisle A, Bay 1, Level 1 | Top shelf |
   | 1 | A-01-02 | Aisle A, Bay 1, Level 2 | Middle |

**Notes:**
- Array length can be less than 48 (only define what you use)
- Pin names must match keys in `table.json` for barcode mapping
- Names are case-sensitive

---

#### `service` (string)

**Description:** Bluetooth LE service UUID for barcode scanner

**Format:** 128-bit UUID (lowercase, with dashes)

**Examples:**
```json
"service": "0000feea-0000-1000-8000-00805f9b34fb"
```

**How to Find:**

1. Check scanner manufacturer documentation
2. Use BLE scanner app (nRF Connect, LightBlue)
3. Check serial monitor - device logs discovered services:
   ```
   S: 0000feea-0000-1000-8000-00805f9b34fb
   ```

**Common Scanner Services:**
- Generic: `0000feea-0000-1000-8000-00805f9b34fb`
- HID over GATT: `00001812-0000-1000-8000-00805f9b34fb`

**Notes:**
- Scanner must advertise this service
- If unsure, leave device to auto-detect
- Must be 128-bit format (even if scanner uses 16-bit)

---

#### `charact` (string)

**Description:** Bluetooth LE characteristic UUID for barcode notifications

**Format:** 128-bit UUID (lowercase, with dashes)

**Examples:**
```json
"charact": "00002aa1-0000-1000-8000-00805f9b34fb"
```

**How to Find:**

1. Check scanner documentation
2. Use BLE scanner app - look for characteristics with "Notify" property
3. Device can auto-detect if left empty

**Properties Required:**
- Must support "Notify" or "Indicate"
- Must be readable

**Notes:**
- Used to receive barcode scan data
- Device subscribes to this characteristic
- If empty, device attempts auto-detection

---

## table.json Reference

### Location

`data/table.json` (uploaded to SPIFFS filesystem)

### Purpose

Maps barcode values to shelf location names (pin names from `config.json`).

### Format

```json
{
    "PinName1": ["barcode1", "barcode2", "barcode3"],
    "PinName2": ["barcode4", "barcode5"],
    "PinName3": ["barcode6"]
}
```

**Structure:**
- **Key:** Pin name (must match entry in `config.json` `pins` array)
- **Value:** Array of barcode strings that should trigger this pin

### Complete Example

```json
{
    "ShelfA-Top": [
        "1234567890",
        "0987654321",
        "1111111111"
    ],
    "ShelfA-Middle": [
        "2222222222",
        "3333333333"
    ],
    "ShelfB-Top": [
        "4444444444"
    ],
    "Route-101": [
        "TRK1001",
        "TRK1002",
        "TRK1003"
    ]
}
```

### Best Practices

#### 1. Validate Barcodes

Ensure barcodes are exact strings as scanned:

```json
// If scanner sends barcode with prefix/suffix, include it
{
    "ShelfA": ["CODE:12345"]  // Not "12345"
}
```

Test by monitoring serial output when scanning:
```
R: CODE:12345 = ShelfA
```

#### 2. Avoid Duplicates

Each barcode should map to only one location:

```json
// Bad - barcode "12345" appears twice
{
    "ShelfA": ["12345", "67890"],
    "ShelfB": ["12345", "11111"]  // Conflict!
}

// Good - unique barcodes
{
    "ShelfA": ["12345", "67890"],
    "ShelfB": ["11111", "22222"]
}
```

**Note:** System returns the first match found.

#### 3. Group Logically

Organize barcodes by category or route:

```json
{
    "Electronics-Bin": [
        "ELEC-001", "ELEC-002", "ELEC-003"
    ],
    "Clothing-Bin": [
        "CLTH-001", "CLTH-002", "CLTH-003"
    ]
}
```

#### 4. Document Large Mappings

For 100+ barcodes, maintain a spreadsheet and generate JSON:

**Spreadsheet:**
| Barcode | Pin Name | Description |
|---------|----------|-------------|
| 1234567890 | A-01-01 | Widget A |
| 0987654321 | A-01-01 | Widget B |
| 1111111111 | A-01-02 | Gadget C |

**Generate with Python:**
```python
import json
import csv

table = {}

with open('mapping.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        pin = row['Pin Name']
        barcode = row['Barcode']

        if pin not in table:
            table[pin] = []
        table[pin].append(barcode)

with open('table.json', 'w') as f:
    json.dump(table, f, indent=4)
```

#### 5. Handle Unknowns

Decide what happens when barcode is not found:
- System logs: `R: [unknown-code] = `
- No LED lights up
- Event sent with empty `pin` field

**Consider:**
- Catch-all bin for unknown items
- Custom code to flash all LEDs for unmapped codes (requires firmware mod)

### Size Limitations

**SPIFFS Filesystem:**
- Total size: ~1.5 MB (depends on partition scheme)
- `table.json` practical limit: ~100 KB
- Approximate capacity: ~5,000 barcode entries

**For larger datasets:**
1. Use external storage (SD card) - requires code modification
2. Use dynamic mapping via API instead of static file
3. Compress/optimize JSON (remove whitespace)
4. Split across multiple devices

### Updating table.json

**Method 1: Re-upload Filesystem**
```bash
# Edit data/table.json
# Then upload
platformio run --target uploadfs
```

**Method 2: Use Web API**
```bash
curl -X POST "http://192.168.4.1/api/writeConfig" \
  -H "Content-Type: application/json" \
  -d @table.json
```

**Note:** Current firmware doesn't have separate endpoint for `table.json` updates. You may need to modify code to add this functionality.

### Validation

Before deploying, validate JSON syntax:

```bash
# Check JSON is valid
python3 -m json.tool data/table.json

# Or use jq
jq . data/table.json
```

**Common Errors:**
```json
// Missing comma
{
    "ShelfA": ["123", "456"]  // ← Missing comma
    "ShelfB": ["789"]
}

// Trailing comma (invalid in JSON)
{
    "ShelfA": ["123", "456"],
    "ShelfB": ["789"],  // ← Remove this comma
}

// Quotes must be double, not single
{
    'ShelfA': ['123']  // ← Use "ShelfA": ["123"]
}
```

---

## Advanced Configuration

### Multiple Devices

When deploying multiple devices, use unique configurations:

**Device 1 (config.json):**
```json
{
    "ssid": "WarehouseWiFi",
    "cidr": "192.168.1.101/24",
    "pins": ["A-01", "A-02", "A-03", ..., "A-48"]
}
```

**Device 2 (config.json):**
```json
{
    "ssid": "WarehouseWiFi",
    "cidr": "192.168.1.102/24",
    "pins": ["B-01", "B-02", "B-03", ..., "B-48"]
}
```

**Shared table.json** (or device-specific):
```json
{
    "A-01": ["barcode1", "barcode2"],
    "A-02": ["barcode3"],
    "B-01": ["barcode4"],
    "B-02": ["barcode5"]
}
```

### Dynamic Configuration

For advanced deployments, configure via API on boot:

**Startup script (runs on external server):**
```python
import requests

device_ip = "192.168.1.101"

# Generate config based on database
config = generate_config_from_database(device_ip)

# Upload to device
requests.post(
    f"http://{device_ip}/api/writeConfig",
    json=config
)
```

### Environment-Specific Configs

Use separate config files for different environments:

```
data/
├── config.json           # Production
├── config.dev.json       # Development
└── config.test.json      # Testing
```

Swap before uploading:
```bash
# Deploy to production
cp data/config.json data/config.bak
cp data/config.prod.json data/config.json
pio run --target uploadfs

# Restore
mv data/config.bak data/config.json
```

---

## Configuration Templates

### Template 1: Small Warehouse (Pick-to-Light)

```json
{
    "standalone": false,
    "ssid": "WarehouseWiFi",
    "wifipass": "YourPassword",
    "cidr": "192.168.1.100/24",
    "gw": "192.168.1.1",
    "addr": "",
    "pins": [
        "A1", "A2", "A3", "A4", "A5", "A6",
        "B1", "B2", "B3", "B4", "B5", "B6",
        "C1", "C2", "C3", "C4", "C5", "C6"
    ],
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

### Template 2: Package Sorting

```json
{
    "standalone": false,
    "ssid": "SortingWiFi",
    "wifipass": "YourPassword",
    "cidr": "10.0.50.10/24",
    "gw": "10.0.50.1",
    "addr": "aa:a8:a2:15:78:d9",
    "pins": [
        "Route-North", "Route-South", "Route-East", "Route-West",
        "Local-1", "Local-2", "Local-3",
        "Priority", "Overnight", "International"
    ],
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

### Template 3: Portable/Demo Unit

```json
{
    "standalone": true,
    "ssid": "PTL-Demo-01",
    "wifipass": "DemoPassword123",
    "addr": "",
    "pins": ["Demo-1", "Demo-2", "Demo-3", "Demo-4"],
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

---

## Troubleshooting Configuration

### WiFi Won't Connect

**Check:**
1. SSID is correct (case-sensitive)
2. Password is correct
3. Network uses WPA/WPA2 (not WPA3 or Enterprise)
4. 2.4GHz network (ESP32 doesn't support 5GHz in most models)

**Test:**
```json
// Try standalone mode first
{
    "standalone": true,
    "ssid": "PTL-Test",
    "wifipass": "12345678"
}
```

### Scanner Won't Pair

**Check:**
1. `addr` matches scanner exactly (lowercase)
2. `service` and `charact` are correct UUIDs
3. Scanner is in pairing mode
4. Scanner is not paired to another device

**Test:**
```json
// Leave empty to see available scanners in serial log
{
    "addr": "",
    "service": "",
    "charact": ""
}
```

### Barcode Doesn't Light LED

**Check:**
1. Pin name in `table.json` matches entry in `config.json` pins array
2. Barcode string matches exactly (check serial output)
3. Barcode doesn't have extra characters

**Debug:**
Monitor serial output when scanning:
```
R: 1234567890 = ShelfA
```

If empty after `=`, barcode not found in `table.json`.

---

## Configuration Security

### Protect Sensitive Data

**WiFi passwords** are stored in plain text:
- Secure physical access to devices
- Use network segmentation (separate VLAN for PTL)
- Consider custom firmware with encrypted config

### Backup Configurations

```bash
# Backup before changes
cp data/config.json backups/config_$(date +%Y%m%d).json
cp data/table.json backups/table_$(date +%Y%m%d).json

# Restore if needed
cp backups/config_20240101.json data/config.json
```

### Version Control

Use Git to track configuration changes:

```bash
git init
git add data/*.json
git commit -m "Initial configuration"

# After changes
git add data/config.json
git commit -m "Updated WiFi SSID for new network"
```

---

**Document Version:** 1.0
**Last Updated:** 2024
