/*
 * PutToLight - LED Blink Control Module
 *
 * Copyright (c) 2023 Serhii Nesterenko
 * Licensed under the MIT License. See LICENSE file in the project root.
 */

#include "DFRobot_CH423.h"
#include "ptl.hpp"
#include <Wire.h>

// CH423 I2C GPIO expander instances (2 chips for 48 total pins)
DFRobot_CH423 *ch423, *ch4231;

// Blink state variables
int currentPin = 0;                    // Currently blinking pin (0-47, 48=all)
Blink blink = BLINK_NONE;              // Current blink state
unsigned long blinkStart = 0;          // Blink sequence start time
unsigned long blinkDuration = 10000UL; // Total blink duration (ms)
int blinkPeriod = 1000;                // Blink cycle period (ms)
int blinkFill = 500;                   // LED on-time per cycle (ms)
bool blinkTest = false;                // Test mode flag

/*
void writeS(int t) {
    digitalWrite(SER_IN, t);
    delay(PIN_DELAY/2);
    digitalWrite(SRCK, HIGH);
    delay(PIN_DELAY);
    digitalWrite(SRCK, LOW);
    delay(PIN_DELAY/2);
}

void writeCodeToLed(int code) {
    digitalWrite(CLR, HIGH);
    digitalWrite(G, HIGH);
    delay(PIN_DELAY);

    for (int i=0;i<16;i++) {
      writeS(code & 1 ? HIGH : LOW);
      code >>= 1;
    }

    //digitalWrite(SER_IN, LOW);

    delay(PIN_DELAY);

    digitalWrite(RCK, HIGH);
    delay(PIN_DELAY);
    digitalWrite(RCK, LOW);
    delay(PIN_DELAY);
    digitalWrite(G, LOW);
}
*/

// Set output level for a specific pin or all pins
// Pin mapping: 0-7=ch423 GPIO, 8-23=ch423 GPO, 24-31=ch4231 GPIO, 32-47=ch4231
// GPO, 48=all pins
void setPin(int x, uint8_t level) {
  if (x < 0 || x > 48)
    return; // 48 - write to all

  // Handle missing first chip
  if (x < 24 && ch423 == nullptr) {
    if (blinkTest) {
      Serial.println("No wire0, set current pin to 24");
      currentPin = 24;
      x = 24;
    } else {
      return;
    }
  }
  // Handle missing second chip
  if (x >= 24 && x < 48 && ch4231 == nullptr) {
    if (blinkTest) {
      Serial.println("No wire1, set current pin to 48");
      currentPin = 48;
      x = 48;
    } else {
      return;
    }
  }
  // First chip GPIO (0-7)
  if (x < 8) {
    ch423->digitalWrite(DFRobot_CH423::eGPIOPin_t(DFRobot_CH423::eGPIO0 + x),
                        level);
    return;
  }
  // First chip GPO (8-23)
  if (x < 24) {
    ch423->digitalWrite(DFRobot_CH423::eGPOPin_t(DFRobot_CH423::eGPO0 + x - 8),
                        level);
    return;
  }
  // Second chip GPIO (24-31)
  if (x < 32) {
    ch4231->digitalWrite(
        DFRobot_CH423::eGPIOPin_t(DFRobot_CH423::eGPIO0 + x - 24), level);
    return;
  }
  // Second chip GPO (32-47)
  if (x < 48) {
    ch4231->digitalWrite(
        DFRobot_CH423::eGPOPin_t(DFRobot_CH423::eGPO0 + x - 32), level);
    return;
  }
  // Write to all pins
  if (level == HIGH)
    level = 0xff;
  if (ch423 != nullptr) {
    ch423->digitalWrite(DFRobot_CH423::eGPIOTotal, level);
    ch423->digitalWrite(DFRobot_CH423::eGPOTotal, level);
  }
  if (ch4231 != nullptr) {
    ch4231->digitalWrite(DFRobot_CH423::eGPIOTotal, level);
    ch4231->digitalWrite(DFRobot_CH423::eGPOTotal, level);
  }
}

// Trigger LED blink sequence for a specific pin
void blinkPin(int pin) {
  currentPin = pin;
  blink = BLINK_START;
}

// LED blink task - runs on dedicated core
void BlinkCode(void *) {
  Serial.printf("Running blink on core %d\n", xPortGetCoreID());

  /*
  int code = 0;

    Serial.print("Init LEDs.. ");

    pinMode(SER_IN, OUTPUT);
    pinMode(SRCK, OUTPUT);
    pinMode(CLR, OUTPUT);
    pinMode(RCK, OUTPUT);
    pinMode(G, OUTPUT);
    //digitalWrite(G, HIGH);
    //digitalWrite(CLR, HIGH);
    digitalWrite(SER_IN, LOW);
    digitalWrite(SRCK, LOW);
    digitalWrite(CLR, LOW);
    digitalWrite(RCK, LOW);
    digitalWrite(G, LOW);
  */

  // Initialize I2C buses and create CH423 instances
  if (Wire.begin())
    ch423 = new DFRobot_CH423(Wire);
  if (Wire1.begin(SDA_2, SCL_2))
    ch4231 = new DFRobot_CH423(Wire1);

  // Test first chip presence
  Wire.beginTransmission(CH423_CMD_SET_SYSTEM_ARGS);
  if (Wire.endTransmission() != 0) {
    Serial.println("Wire0 not found!");
    delete ch423;
    ch423 = nullptr;
  } else {
    ch423->begin();
    ch423->pinMode(DFRobot_CH423::eGPO, DFRobot_CH423::ePUSH_PULL);
    ch423->pinMode(DFRobot_CH423::eGPIO, DFRobot_CH423::eOUTPUT);
  }

  // Test second chip presence
  Wire1.beginTransmission(CH423_CMD_SET_SYSTEM_ARGS);
  if (Wire1.endTransmission() != 0) {
    Serial.println("Wire1 not found!");
    delete ch4231;
    ch4231 = nullptr;
  } else {
    ch4231->begin();
    ch4231->pinMode(DFRobot_CH423::eGPO, DFRobot_CH423::ePUSH_PULL);
    ch4231->pinMode(DFRobot_CH423::eGPIO, DFRobot_CH423::eOUTPUT);
  }

  setPin(48, HIGH); // Turn off all LEDs

  // Main blink loop
  for (;;) {
    blinkLoop();
    vTaskDelay(10);
  }
}

unsigned long lastBlink = 0; // Last blink state change time

// LED blink state machine - handles blinking patterns
void blinkLoop() {
  // Start new blink sequence
  if (blink == BLINK_START) {
    Serial.printf("Starting to blink %d\n", currentPin);
    blinkStart = millis();
    lastBlink = 0;
    blink = BLINK_HIGH;
    setPin(48, HIGH); // Turn off all other LEDs
  }
  // Check if blink duration expired
  if (blink != BLINK_NONE && millis() - blinkStart > blinkDuration) {
    if (blinkTest) {
      currentPin++;
      blink = BLINK_START;
    }
    if (!blinkTest || currentPin > 48) {
      blink = BLINK_NONE;
      blinkTest = false;
      setPin(48, HIGH); // Turn off all LEDs
    }
  }
  // LED off period complete, turn LED on
  if (blink == BLINK_LOW && millis() - lastBlink > blinkFill) {
    setPin(currentPin, HIGH);
    lastBlink = millis();
    blink = BLINK_HIGH;
  }
  // LED on period complete, turn LED off
  if (blink == BLINK_HIGH && millis() - lastBlink > blinkPeriod - blinkFill) {
    setPin(currentPin, LOW);
    lastBlink = millis();
    blink = BLINK_LOW;
  }
  /*
  if (code != codeTarget) {
        writeCodeToLed(codeTarget);
        code = codeTarget;
      }
  */
}
