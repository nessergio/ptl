/*
 * PutToLight - Time and Logging Module
 *
 * Copyright (c) 2025 Serhii Nesterenko
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "ptl.hpp"
#include <time.h>

#define LOG_MAX 100  // Maximum log entries in circular buffer

// NTP time synchronization settings
const char* ntpServer = "pool.ntp.org";        // NTP server address
const long  gmtOffset_sec = 2*60*60;           // GMT+2 offset (seconds)
const int   daylightOffset_sec = 3600;         // Daylight saving time offset (1 hour)

// Circular log buffer
log_t *pLog = new log_t[LOG_MAX];
int beg = 0, end = 0;  // Buffer start and end indices

// Get current Unix timestamp from ESP32 RTC
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

// Add entry to circular log buffer
bool putToLog(string pin, string code) {
    pLog[end].pin = pin;
    pLog[end].code = code;
    pLog[end].t = getTime();
    if (++end > LOG_MAX) {
        end = 0;  // Wrap around to start
    }
    return true;
}

// Initialize NTP time synchronization
void initLog() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}   