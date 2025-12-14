#include <ArduinoJson.h>

// Shift register pins
#define SER_IN 13  // Serial data input
#define SRCK   12  // Shift register clock
#define CLR    16  // Clear/reset pin
#define RCK    4   // Register clock (latch)
#define G      2   // Output enable (active low)

// I2C secondary bus pins
#define SDA_2 18  // Secondary I2C data line
#define SCL_2 19  // Secondary I2C clock line

// Timing constants
#define PIN_DELAY 1      // Delay between pin operations (ms)
#define SCAN_TIME 5      // BLE scan duration (seconds)
#define MAX_DEVICES 20   // Maximum number of BLE devices to track
#define MAX_SCAN 100     // Maximum scan buffer size

// CH423 I2C command
#define CH423_CMD_SET_SYSTEM_ARGS   (0x48 >> 1)

using namespace std::__cxx11;

// Device connection status
enum Status {
  STATUS_INIT,                   // Initializing
  STATUS_DEVICE_NOT_CONNECTED,   // No device connected
  STATUS_DEVICE_CONNECTED        // Device connected
};

// LED blink states
enum Blink {
  BLINK_NONE,   // Not blinking
  BLINK_START,  // Start blinking sequence
  BLINK_HIGH,   // LED on state
  BLINK_LOW     // LED off state
};

// BLE scan states
enum ScanMode {
  SCAN_NONE,        // Not scanning
  SCAN_IN_PROGRESS, // Currently scanning
  SCAN_FINISHED     // Scan complete
};

// BLE device information
typedef struct {
  string address;  // Device MAC address
  string service;  // Device service UUID
} device_t;

// Access log entry
typedef struct {
  unsigned long t;  // Timestamp
  string pin;       // Pin/location identifier
  string code;      // Access code/identifier
} log_t;

// Global device storage
extern device_t devices[MAX_DEVICES];  // Array of discovered BLE devices
extern int nDevices;                   // Number of devices found

// Global status variables
extern Status status;              // Current connection status
extern char scan[MAX_SCAN];        // Scan result buffer
extern ScanMode scanMode;          // Current scan state

// Task entry points
extern void BLECode(void *params);    // BLE scanning task
extern void BlinkCode(void *params);  // LED blinking task

// Access control
extern int codeTarget;  // Target code for comparison

// Timing
extern unsigned long timerDelay;  // Timer delay value

// Configuration
extern DynamicJsonDocument cfg;  // JSON configuration document

// Initialization and main loop functions
void initBlink();              // Initialize LED blink system
void blinkLoop();              // Process LED blink state machine
void initLog();                // Initialize logging system
void blinkPin(int pin);        // Trigger LED blink on specific pin

// Utility functions
unsigned long getTime();          // Get current timestamp
void disconnectFromScanner();     // Disconnect from BLE scanner