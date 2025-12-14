/*
 * PutToLight - Main Application File
 *
 * Copyright (c) 2025 Serhii Nesterenko
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "SPIFFS.h"
#include "ptl.hpp"
#include <Adafruit_NeoPixel.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

// Timing and status globals
unsigned long timerDelay = 10000;          // Status update interval (ms)
unsigned long lastRead = 0, lastWrite = 0; // Last config read/write times

// Web server and SSE event source
AsyncWebServer server(80);
AsyncEventSource events("/events"); // Server-sent events for real-time updates

Status status = STATUS_INIT; // Current connection status

// NeoPixel LED strip configuration
#define DATA_PIN 16
const int numOfLeds = 60;
Adafruit_NeoPixel strip =
    Adafruit_NeoPixel(numOfLeds, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Serial buffer (unused)
char lineBuf[80];
int charCount = 0;

bool fileWritten = false; // Config file write success flag

// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi - either as station or access point
void initWiFi() {
  IPAddress IP;
  if (cfg["standalone"] == false) {
    // Station mode - connect to existing WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char *)cfg["ssid"], (const char *)cfg["wifipass"]);
    Serial.printf("Connecting to WiFi (%s)\n", (const char *)cfg["ssid"]);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
    IP = WiFi.localIP();
  } else {
    // Access Point mode - create own network
    WiFi.mode(WIFI_AP);
    Serial.printf("Setting AP (%s)…\n", (const char *)cfg["ssid"]);
    WiFi.softAP((const char *)cfg["ssid"], (const char *)cfg["wifipass"]);
    IP = WiFi.softAPIP();
  }
  Serial.println(IP);
}

// FreeRTOS task handles
TaskHandle_t Task1, Task2;

// JSON document buffers
char buf[1024];                                      // Serialization buffer
DynamicJsonDocument cfg = DynamicJsonDocument(1024); // Configuration
DynamicJsonDocument tbl =
    DynamicJsonDocument(4096); // Access table (code->pin mapping)
DynamicJsonDocument doc =
    DynamicJsonDocument(1024); // Temporary document for responses

// Send status update via Server-Sent Events
void sendStatus() {
  JsonObject json = doc.to<JsonObject>();
  json["status"] = status;
  json["r"] = lastRead;
  json["w"] = lastWrite;
  JsonArray array = json.createNestedArray("devices");
  for (int i = 0; i < nDevices; i++) {
    const JsonObject &d = array.createNestedObject();
    d["address"] = devices[i].address;
    d["service"] = devices[i].service;
  }
  serializeJson(doc, buf);
  events.send(buf, "status", millis());
}

// Load configuration from SPIFFS
void readConfig() {
  File file = SPIFFS.open("/config.json", FILE_READ);
  if (!file) {
    Serial.println("ERROR: There was an error opening config file");
    return;
  }
  Serial.println("Config opened!");
  DeserializationError error = deserializeJson(cfg, file);
  file.close();
  if (error) {
    Serial.println("ERROR: deserialize");
    return;
  }
  lastWrite = getTime();
}

// Load access control table from SPIFFS (maps codes to pin names)
void readTable() {
  File file = SPIFFS.open("/table.json", FILE_READ);
  if (!file) {
    Serial.println("ERROR: There was an error opening table file");
    return;
  }
  Serial.println("Table opened!");
  DeserializationError error = deserializeJson(tbl, file);
  file.close();
  if (error) {
    Serial.println("ERROR: deserialize");
    return;
  }
  lastRead = getTime();
}

// 404 handler
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Handle config file upload body
void handleWriteConfigBody(AsyncWebServerRequest *request, uint8_t *data,
                           size_t len, size_t index, size_t total) {
  if (data == nullptr || len != total) {
    return;
  }

  File file = SPIFFS.open("/config.json", FILE_WRITE);
  if (!file) {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  if (file.write(data, len) != 0) {
    fileWritten = true;
  }
  file.close();

  Serial.println("WRITTEN CONFIG:");
  Serial.write(data, len);
  Serial.println();
}

// Handle config write completion - optionally reboot
void handleWriteConfig(AsyncWebServerRequest *request) {
  bool reboot = false;
  AsyncWebParameter *rebootParam = request->getParam("reboot");

  if (rebootParam != nullptr && rebootParam->value() == "true") {
    reboot = true;
    Serial.println("REBOOT ISSUED!");
  }

  if (fileWritten) {
    request->send(200, "application/json", "{}");
  } else {
    request->send(500, "application/json", "{msg:'unable to write file'}");
  }

  if (reboot)
    ESP.restart();
  else
    readConfig();
}

uint8_t *b; // Index page buffer (unused)
int b_len = 0;
void getIndex(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response =
      request->beginResponse_P(200, "text/html", b, b_len);
  request->send(response);
}

// Look up pin name for a given scan code in access table
JsonString findInTable(JsonString x) {
  for (JsonPair kv : tbl.as<JsonObject>()) {
    for (JsonVariant value : kv.value().as<JsonArray>()) {
      if (value.as<JsonString>() == x)
        return kv.key(); // Return pin name
    }
  }
  return JsonString(""); // Not found
}

// Find pin number for a given pin name
int findPin(JsonString pinName) {
  int i = 0;
  for (JsonVariant value : cfg["pins"].as<JsonArray>()) {
    if (value.as<JsonString>() == pinName)
      return i;
    i++;
  }
  return 48; // Default to "all pins"
}

// STM32 setup function - initialize system
void setup() {
  Serial.begin(115200);

  // Initialize file system and load configuration
  initSPIFFS();
  readConfig();

  // Create FreeRTOS tasks on separate cores
  xTaskCreatePinnedToCore(&BLECode, "BT", 5000, NULL, 15, &Task1,
                          0); // BLE task on core 0
  xTaskCreatePinnedToCore(&BlinkCode, "Blink", 5000, NULL, 10, &Task2,
                          0); // Blink task on core 0

  // Initialize peripherals
  initWiFi();
  strip.begin(); // Initialize NeoPixel strip
  initLog();     // Initialize NTP time sync

  readTable(); // Load access control table

  /*File file = SPIFFS.open("/index.html", FILE_READ);
  b_len = file.available();
  b = new uint8_t[b_len];
  file.read((byte *)b, b_len);
  file.close();*/

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Web Server Root URL
  // server.on("/", HTTP_GET, &getIndex);
  /*
      // CONFIG
      server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/config.json", "application/json");
      });
      */

  server.on("/api/wifiInfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.printf("Running server on core %d\n", xPortGetCoreID());
    AsyncResponseStream *response =
        request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["status"] = "ok";
    json["ssid"] = WiFi.SSID();
    json["gw"] = WiFi.gatewayIP().toString();
    json["dns"] = WiFi.dnsIP().toString();
    json["cidr"] = WiFi.localIP().toString() + "/" + WiFi.subnetCIDR();
    serializeJson(json, *response);
    request->send(response);
  });

  server.on("/api/writeConfig", HTTP_POST, handleWriteConfig, NULL,
            handleWriteConfigBody);

  server.addHandler(new AsyncCallbackJsonWebHandler(
      "/api/blink", [](AsyncWebServerRequest *request, JsonVariant &json) {
        const JsonObject &jsonObj = json.as<JsonObject>();
        int pin = jsonObj["pin"];
        Serial.printf("Command to blink: %d \n", pin);
        blinkPin(pin);
        request->send(200);
      }));

  server.addHandler(new AsyncCallbackJsonWebHandler(
      "/api/setLed", [](AsyncWebServerRequest *request, JsonVariant &json) {
        const JsonObject &jsonObj = json.as<JsonObject>();
        AsyncResponseStream *response =
            request->beginResponseStream("application/json");
        DynamicJsonDocument root(1024);
        Serial.print("Reading: ");
        const char *code = jsonObj["code"];
        Serial.println(code);
        if (code != nullptr) {
          codeTarget = strtol(code, NULL, 0);
          Serial.print("Code: ");
          Serial.println(codeTarget);
        }
        root["test"] = jsonObj["helo"]; // ESP.getFreeHeap();
        root["ssid"] = WiFi.SSID();
        serializeJson(root, *response);
        request->send(response);
      }));

  server.addHandler(new AsyncCallbackJsonWebHandler(
      "/api/setDevice", [](AsyncWebServerRequest *request, JsonVariant &json) {
        const JsonObject &jsonObj = json.as<JsonObject>();
        Serial.print("Want device: ");
        cfg["addr"] = jsonObj["address"] != nullptr
                          ? string((const char *)jsonObj["address"])
                          : "";
        cfg["service"] = jsonObj["service"] != nullptr
                             ? string((const char *)jsonObj["service"])
                             : "";
        cfg["charact"] = jsonObj["charact"] != nullptr
                             ? string((const char *)jsonObj["charact"])
                             : "";
        Serial.printf("%s/%s/%s\n", (const char *)cfg["addr"],
                      (const char *)cfg["service"],
                      (const char *)cfg["charact"]);
        disconnectFromScanner();
        request->send(200);
      }));

  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it gat is: %u\n",
                    client->lastId());
    }
    // send event with message "hello!", id current millis and set reconnect
    // delay to 1 second
    client->send("hello!", "open", millis(), 1000);
  });
  server.addHandler(&events);

  server.onNotFound(notFound);

  // initBlink();
  //  Start server
  server.begin();
  Serial.printf("Running main on core %d\n", xPortGetCoreID());
}

unsigned long lastTime = 0; // Last status update time

// Process received scan from BLE device - look up pin and trigger blink
void processScan(const char *scan) {
  JsonString pinName = findInTable(scan); // Look up pin name for this code
  blinkPin(findPin(JsonString(pinName))); // Trigger LED blink
  Serial.printf("R: %s = %s\n", scan, pinName);
  // Send scan event to web clients
  JsonObject json = doc.to<JsonObject>();
  json["code"] = scan;
  json["pin"] = pinName;
  json["t"] = getTime();
  serializeJson(doc, buf);
  events.send(buf, "scan", millis());
}

// STM32 main loop
void loop() {

  // Random NeoPixel animation
  for (int x = 0; x < numOfLeds; x++) {
    int g = random(0, 255);
    int r = random(0, 255);
    int b = random(0, 255);
    strip.setPixelColor(x, r, g, b);
  }
  strip.show();

  // Periodic status updates
  if ((millis() - lastTime) > timerDelay) {
    Serial.printf("HEAP: %d\n", ESP.getFreeHeap());
    sendStatus();
    lastTime = millis();
  }
  // Process completed scans
  if (scanMode == SCAN_FINISHED) {
    scanMode = SCAN_NONE;
    processScan(scan);
  }
  // Handle WiFi reconnection
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.reconnect();
  }
  vTaskDelay(1000);
}
