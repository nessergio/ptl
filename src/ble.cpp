#include "NimBLEDevice.h"
#include "NimBLEScan.h"
#include "ptl.hpp"

using namespace std::__cxx11;

// BLE globals
int codeTarget = 0x5555;           // Target code value (unused)
device_t devices[MAX_DEVICES];     // Array of discovered BLE devices
int nDevices = 0;                  // Number of devices found
BLEScan* pBLEScan;                 // BLE scanner instance
char scan[MAX_SCAN];               // Scan data buffer
int scanPos = 0;                   // Current position in scan buffer
ScanMode scanMode = SCAN_NONE;     // Current scan state

// BLE client and characteristic handles
NimBLEClient *pClient = BLEDevice::createClient();
NimBLERemoteCharacteristic *pChar = nullptr;

/*class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s; ", advertisedDevice.toString().c_str());
      Serial.printf("%s", advertisedDevice.getServiceUUID().toString().c_str());
      int nServices = advertisedDevice.getServiceDataCount();
      Serial.printf("Has %d services \n", nServices);
      const char *addr = advertisedDevice.getAddress().toString().c_str();

      /*for (int i=0;i<nDevices;i++) {
        if (!strcmp(addr, devices[i].address)) return;
      }
      if (nDevices >= MAX_DEVICES) return;*/
      
      /*memcpy(&devices[nDevices].address, addr, sizeof(devices->address));
      devices[nDevices].name = string(advertisedDevice.getName().c_str());
      if (devices[nDevices].name.length() == 0) {
        devices[nDevices].name = devices[nDevices].address;
      }*//*
      if (advertisedDevice.getAddress() == deviceTarget) {
        status = STATUS_CONNECTING_DEVICE;
      }
      //devices[nDevices].service = advertisedDevice.getServiceUUID().toString();
      //if (!strcmp(devices[nDevices].service.c_str(), "<NULL>")) devices[nDevices].service = "";
      //nDevices++;
    }
};*/

// BLE notification callback - receives scan data from connected device
// Data arrives in chunks until CR (13) is received
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    scanMode = SCAN_IN_PROGRESS;
    // Prevent buffer overflow
    if (scanPos + length > MAX_SCAN) {
      length = MAX_SCAN - scanPos;
      pData[length - 1] = 13;  // Force end of scan
    }
    strncpy(&scan[scanPos], (const char *)pData, length);
    scanPos += length;
    // End of line (CR)? Mark scan as complete
    if (pData[length-1] == 13) {
      scan[scanPos-1] = 0;  // Null terminate
      scanPos = 0;
      scanMode = SCAN_FINISHED;
    }
}

// Subscribe to BLE characteristic notifications
bool subscribeCharacteristic() {
  if (pChar == nullptr) {
    Serial.println("ERROR GETTING CHARACTERISTIC");
    return false;
  }
  if (!pChar->canRead()) {
    Serial.println("CHARACTERISTIC IS NOT READABLE");
    return false;
  }
  if (!pChar->canNotify()) {
    Serial.println("CHARACTERISTIC IS NOT NOTIFYABLE");
    return false;
  }
  cfg["charact"] = pChar->getUUID().to128().toString();
  pChar->subscribe(false, notifyCallback);  // Subscribe for indications
  Serial.println("SUBSCRIBED");
  return true;
}

// Connect to BLE scanner device and subscribe to scan characteristic
bool connectToScanner() {
  if (cfg["addr"] == "")
    return false;
  Serial.print("Connecting to ");
  Serial.println((const char *)cfg["addr"]);
  // Try public address first, then random address
  if (!pClient->connect(NimBLEAddress((const char *)cfg["addr"]))) {
    if (!pClient->connect(NimBLEAddress((const char *)cfg["addr"]), BLE_ADDR_RANDOM)) {
      Serial.println("ERROR CONNECTING OT DEVICE");
      return false;
    }
  }
  Serial.println("CONNECTED TO DEVICE");
    //Serial.println("2");
    /*std::map<std::string, BLERemoteService*> *foundServices = pClient->getServices();
    if (foundServices == nullptr) {
      status = STATUS_DONE_SCANNING;
      Serial.println("ERROR");
      return;
    }
    Serial.println("3");
    for (auto it = foundServices->begin(); it != foundServices->end(); it++) {
      Serial.print("1: ");
      Serial.print(it->first.c_str());
      Serial.print(" 2: ");
      Serial.print(it->second->getUUID().toString().c_str());
      Serial.println();
    }*/
  // Get the configured service
  NimBLERemoteService *srv = pClient->getService(NimBLEUUID((const char *)cfg["service"]));
  if (srv == nullptr) {
    Serial.println("ERROR CONNECTING TO SERVICE");
    return false;
  }
  Serial.println("CONNECTED TO SERVICE");
  // If we have a specific characteristic configured, subscribe to it
  if (cfg["charact"].as<string>().length() > 0) {
    pChar = srv->getCharacteristic(NimBLEUUID((const char *)cfg["charact"]));
    if (subscribeCharacteristic()) return true;
  }
    /*std::map<std::string, NimBLERemoteCharacteristic*> *pChars = srv->getCharacteristics();
    if (pChars == nullptr) { 
      Serial.println("ERROR GETTING CHARACTERISTICS");
      return false;
    }
    Serial.println("GOT CHARACTERISTICS:");
    for (auto it=pChars->begin(); it!=pChars->end(); it++) { 
      Serial.printf("%d ", it->second->canNotify());
      Serial.println(it->second->getUUID().toString().c_str());
      if (it->second->canRead() && it->second->canNotify()) {
        subscribeCharacteristic(it->second);
        return true;
      }
    }*/   
  Serial.println("NO VALUABLE CHARACTERISTIC FOUND");
  return false;
}

// Initialize BLE subsystem and scanner
void initBLE() {
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();  // Create new scan instance
    pBLEScan->setInterval(150);  // Set scan interval (ms)
    pBLEScan->setWindow(50);     // Set scan window (ms)
    Serial.println("Init BLE ok");
}

// Find device by address in discovered devices list
int findDevice(string address) {
  for (int i=0; i<nDevices; i++) {
    if (address == devices[i].address)
      return i;
  }
  return -1;  // Not found
}

// Scan for BLE devices and look for configured target device
bool scanBLE() {
  Serial.println("Scanning BLE...");
  NimBLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
  for(int i = 0; i < foundDevices.getCount(); i++) {
    NimBLEAdvertisedDevice device = foundDevices.getDevice(i);

    // Add new devices to list
    if (findDevice(device.getAddress().toString()) == -1 && nDevices < MAX_DEVICES) {
      devices[nDevices].address = device.getAddress().toString();
      devices[nDevices].service = device.getServiceUUID().to128().toString();
      nDevices++;
    }

    Serial.printf("A: %s N: %s S: %s RSSI: %d M: %X, X: %X\n",
      device.getAddress().toString().c_str(),
      device.getName().c_str(),
      device.getServiceUUID().toString().c_str(),
      device.getRSSI(),
      device.getManufacturerData(),
      device.getPayload()
    );
    // Check if this is our target device
    if (device.getAddress().toString() == cfg["addr"].as<string>()) {
      Serial.println("FOUND MY DEVICE!");
      return true;
    }
  }
  return false;
}

// Disconnect from BLE scanner device
void disconnectFromScanner() {
  status = STATUS_DEVICE_NOT_CONNECTED;
  pClient->deleteServices();
  pClient->disconnect();
}

// BLE task - scans for and maintains connection to scanner device
void BLECode(void *params) {
    Serial.printf("Running ble on core %d\n", xPortGetCoreID());

    initBLE();

    if (pClient == nullptr) {
      Serial.println("Can not create client!");
      return;
    }

    Serial.println();
    Serial.print("DONE ");

    unsigned long lastTime = 0;

    // Main BLE loop - periodically scan and reconnect
    for (;;) {

      if ((millis() - lastTime) > timerDelay) {
        if (!pClient->isConnected()) {
          disconnectFromScanner();
          // Scan for device and try to connect
          if (scanBLE() && connectToScanner()) {
            Serial.printf("Connected to %s service %s\n", (const char *)cfg["addr"], (const char *)cfg["service"]);
            status = STATUS_DEVICE_CONNECTED;
          }
        }
        lastTime = millis();
      }

      delay(1000);
    }

}