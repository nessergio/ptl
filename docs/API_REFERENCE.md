# API Reference

Complete REST API and WebSocket documentation for the Pick-to-Light system.

## Base URL

```
http://[device-ip-address]
```

Default IPs:
- AP Mode: `http://192.168.4.1`
- Station Mode: Check serial monitor or your router's DHCP table

## Authentication

Currently, the API does not require authentication. Consider implementing authentication if deploying in production environments.

## REST API Endpoints

### WiFi Information

Get current WiFi connection details.

**Endpoint:** `GET /api/wifiInfo`

**Response:**
```json
{
    "status": "ok",
    "ssid": "MyNetwork",
    "gw": "192.168.1.1",
    "dns": "192.168.1.1",
    "cidr": "192.168.1.100/24"
}
```

**Response Fields:**
| Field | Type | Description |
|-------|------|-------------|
| status | string | Always "ok" if responding |
| ssid | string | Connected WiFi network name |
| gw | string | Gateway IP address |
| dns | string | DNS server IP address |
| cidr | string | Device IP with subnet mask in CIDR notation |

**Example:**
```bash
curl http://192.168.4.1/api/wifiInfo
```

---

### Set BLE Device

Configure which Bluetooth barcode scanner to connect to.

**Endpoint:** `POST /api/setDevice`

**Request Body:**
```json
{
    "address": "aa:a8:a2:15:78:d9",
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

**Request Fields:**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| address | string | Yes | Bluetooth MAC address of scanner (lowercase, colon-separated) |
| service | string | No | BLE service UUID (128-bit format) |
| charact | string | No | BLE characteristic UUID for notifications |

**Response:**
- Status: `200 OK` (no body)
- Device will disconnect from current scanner and attempt to connect to new one

**Notes:**
- Triggers immediate disconnection from current scanner
- Device will attempt reconnection on next scan cycle (every 10 seconds)
- MAC address is case-sensitive (use lowercase)

**Example:**
```bash
curl -X POST http://192.168.4.1/api/setDevice \
  -H "Content-Type: application/json" \
  -d '{
    "address": "aa:a8:a2:15:78:d9",
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
  }'
```

---

### Manual LED Blink

Manually trigger LED blinking for testing or manual operations.

**Endpoint:** `POST /api/blink`

**Request Body:**
```json
{
    "pin": 5
}
```

**Request Fields:**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| pin | integer | Yes | Pin number (0-47) or 48 for all LEDs |

**Pin Mapping:**
- `0-7`: CH423 #1 GPIO pins
- `8-23`: CH423 #1 GPO pins
- `24-31`: CH423 #2 GPIO pins
- `32-47`: CH423 #2 GPO pins
- `48`: Special value - activates ALL LEDs simultaneously

**Response:**
- Status: `200 OK` (no body)
- LED will blink for configured duration (default: 10 seconds)

**Example:**
```bash
# Blink LED on pin 5
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 5}'

# Blink all LEDs
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 48}'
```

---

### Set LED by Code (Legacy)

Trigger LED based on barcode value (legacy endpoint, primarily for debugging).

**Endpoint:** `POST /api/setLed`

**Request Body:**
```json
{
    "code": "1234567890",
    "helo": "test"
}
```

**Request Fields:**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| code | string | No | Barcode value (sets internal target code) |
| helo | any | No | Test field echoed back in response |

**Response:**
```json
{
    "test": "test",
    "ssid": "MyNetwork"
}
```

**Notes:**
- This endpoint is primarily for debugging
- Use `/api/blink` for manual LED control
- Barcode scanning via BLE is the primary use case

**Example:**
```bash
curl -X POST http://192.168.4.1/api/setLed \
  -H "Content-Type: application/json" \
  -d '{"code": "1234567890", "helo": "test"}'
```

---

### Write Configuration

Update device configuration and optionally reboot.

**Endpoint:** `POST /api/writeConfig?reboot=true`

**Query Parameters:**
| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| reboot | boolean | No | Set to "true" to restart device after writing config |

**Request Body:**

Complete `config.json` structure:
```json
{
    "standalone": false,
    "ssid": "MyNetwork",
    "wifipass": "MyPassword123",
    "cidr": "192.168.88.201/24",
    "gw": "192.168.88.1",
    "addr": "aa:a8:a2:15:78:d9",
    "pins": ["ShelfA", "ShelfB", "ShelfC"],
    "service": "0000feea-0000-1000-8000-00805f9b34fb",
    "charact": "00002aa1-0000-1000-8000-00805f9b34fb"
}
```

**Response:**
```json
{}
```

**Error Response:**
```json
{
    "msg": "unable to write file"
}
```

**Notes:**
- Configuration is written to SPIFFS filesystem
- If `reboot=true`, device restarts immediately after write
- If no reboot, configuration is reloaded without restart (WiFi settings require reboot)

**Example:**
```bash
# Write config and reboot
curl -X POST "http://192.168.4.1/api/writeConfig?reboot=true" \
  -H "Content-Type: application/json" \
  -d @config.json

# Write config without reboot
curl -X POST "http://192.168.4.1/api/writeConfig" \
  -H "Content-Type: application/json" \
  -d '{
    "standalone": false,
    "ssid": "NewNetwork",
    "wifipass": "NewPassword"
  }'
```

---

## Server-Sent Events (SSE)

Real-time event stream for monitoring device status and scans.

**Endpoint:** `GET /events`

**Connection:**
```javascript
const eventSource = new EventSource('http://192.168.4.1/events');
```

### Event Types

#### 1. Open Event

Sent when client connects or reconnects.

**Event:** `open`

**Data:**
```json
"hello!"
```

**Example Handler:**
```javascript
eventSource.addEventListener('open', (event) => {
    console.log('Connected:', event.data);
});
```

---

#### 2. Status Event

Periodic status updates (every 10 seconds).

**Event:** `status`

**Data:**
```json
{
    "status": 1,
    "r": 1702834567,
    "w": 1702834567,
    "devices": [
        {
            "address": "aa:a8:a2:15:78:d9",
            "service": "0000feea-0000-1000-8000-00805f9b34fb"
        }
    ]
}
```

**Fields:**
| Field | Type | Description |
|-------|------|-------------|
| status | integer | Connection status: 0=init, 1=not connected, 2=connected |
| r | integer | Last read timestamp (Unix epoch) |
| w | integer | Last write timestamp (Unix epoch) |
| devices | array | List of discovered BLE devices |

**Status Values:**
- `0` (`STATUS_INIT`): Device initializing
- `1` (`STATUS_DEVICE_NOT_CONNECTED`): Scanner not connected
- `2` (`STATUS_DEVICE_CONNECTED`): Scanner connected and ready

**Example Handler:**
```javascript
eventSource.addEventListener('status', (event) => {
    const data = JSON.parse(event.data);
    console.log('Device status:', data.status);
    console.log('BLE devices:', data.devices);

    if (data.status === 2) {
        console.log('Scanner ready!');
    }
});
```

---

#### 3. Scan Event

Triggered when a barcode is scanned.

**Event:** `scan`

**Data:**
```json
{
    "code": "1234567890",
    "pin": "ShelfA",
    "t": 1702834567
}
```

**Fields:**
| Field | Type | Description |
|-------|------|-------------|
| code | string | Scanned barcode value |
| pin | string | Mapped pin name from table.json (empty if not found) |
| t | integer | Scan timestamp (Unix epoch) |

**Example Handler:**
```javascript
eventSource.addEventListener('scan', (event) => {
    const data = JSON.parse(event.data);
    console.log(`Scanned: ${data.code}`);
    console.log(`Shelf: ${data.pin}`);
    console.log(`Time: ${new Date(data.t * 1000)}`);

    if (data.pin) {
        // Barcode was mapped to a shelf
        updateUI(`Highlight ${data.pin}`);
    } else {
        // Barcode not found in mapping
        showError(`Unknown barcode: ${data.code}`);
    }
});
```

---

### Complete SSE Example

```javascript
const eventSource = new EventSource('http://192.168.4.1/events');

// Connection opened
eventSource.addEventListener('open', (event) => {
    console.log('Connected to device');
    updateConnectionStatus('connected');
});

// Device status updates
eventSource.addEventListener('status', (event) => {
    const status = JSON.parse(event.data);

    // Update scanner connection status
    const scannerStatus = document.getElementById('scanner-status');
    if (status.status === 2) {
        scannerStatus.textContent = 'Connected';
        scannerStatus.className = 'connected';
    } else {
        scannerStatus.textContent = 'Not Connected';
        scannerStatus.className = 'disconnected';
    }

    // Update device list
    updateDeviceList(status.devices);
});

// Barcode scan events
eventSource.addEventListener('scan', (event) => {
    const scan = JSON.parse(event.data);

    // Log to scan history
    addToScanHistory(scan.code, scan.pin, new Date(scan.t * 1000));

    // Highlight shelf visually
    if (scan.pin) {
        highlightShelf(scan.pin);
    } else {
        showNotification(`Unknown barcode: ${scan.code}`, 'warning');
    }
});

// Handle errors
eventSource.onerror = (error) => {
    console.error('EventSource error:', error);
    updateConnectionStatus('error');
};

// Close connection when page unloads
window.addEventListener('beforeunload', () => {
    eventSource.close();
});
```

---

## Static Files

### Web Interface

**Endpoint:** `GET /`

Returns the main web interface HTML page.

**Example:**
```
http://192.168.4.1/
```

---

### Configuration Files

**Note:** Direct access to configuration files via HTTP is disabled for security. Use the `/api/writeConfig` endpoint instead.

---

## Error Responses

### Standard HTTP Errors

| Status Code | Description |
|-------------|-------------|
| 200 OK | Request successful |
| 404 Not Found | Endpoint or resource not found |
| 500 Internal Server Error | Server error (check serial logs) |

### Custom Error Messages

Configuration write errors:
```json
{
    "msg": "unable to write file"
}
```

---

## Rate Limits

No rate limiting is currently implemented. Consider implementing rate limiting for production deployments.

---

## WebSocket Alternative

Currently, the system uses Server-Sent Events (SSE) for real-time updates. SSE is simpler than WebSocket for unidirectional server-to-client communication and works well for this use case.

**Advantages of SSE:**
- Automatic reconnection
- Built-in event ID tracking
- Simpler implementation
- Works over standard HTTP

---

## Integration Examples

### Python

```python
import requests
import sseclient

# Get WiFi info
response = requests.get('http://192.168.4.1/api/wifiInfo')
print(response.json())

# Blink LED
requests.post('http://192.168.4.1/api/blink',
              json={'pin': 5})

# Listen to events
response = requests.get('http://192.168.4.1/events', stream=True)
client = sseclient.SSEClient(response)

for event in client.events():
    if event.event == 'scan':
        data = json.loads(event.data)
        print(f"Scanned: {data['code']} -> {data['pin']}")
```

### Node.js

```javascript
const axios = require('axios');
const EventSource = require('eventsource');

// Get WiFi info
axios.get('http://192.168.4.1/api/wifiInfo')
    .then(response => console.log(response.data));

// Blink LED
axios.post('http://192.168.4.1/api/blink', { pin: 5 });

// Listen to events
const eventSource = new EventSource('http://192.168.4.1/events');

eventSource.addEventListener('scan', (event) => {
    const data = JSON.parse(event.data);
    console.log(`Scanned: ${data.code} -> ${data.pin}`);
});
```

### cURL

```bash
# Get status
curl http://192.168.4.1/api/wifiInfo

# Blink LED
curl -X POST http://192.168.4.1/api/blink \
  -H "Content-Type: application/json" \
  -d '{"pin": 5}'

# Stream events
curl -N http://192.168.4.1/events
```

---

## Future API Enhancements

Planned features for future versions:

1. **Authentication**: JWT or API key based authentication
2. **HTTPS**: SSL/TLS support
3. **WebSocket**: Optional WebSocket support for bidirectional communication
4. **Bulk Operations**: Endpoint for controlling multiple LEDs
5. **Analytics**: Historical scan data and statistics
6. **Configuration Validation**: Validate config before applying
7. **Firmware Updates**: OTA (Over-The-Air) update endpoint

---

## Support

For issues or questions about the API:
- Check serial monitor logs (115200 baud)
- Review [README.md](../README.md) for general information
- See [QUICK_START.md](QUICK_START.md) for basic usage
