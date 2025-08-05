// File: ButtonsAndLEDs.h
#ifndef BUTTONS_AND_LEDS_H
#define BUTTONS_AND_LEDS_H

#include <Arduino.h>
#include <EEPROM.h>
#include "MIDIHandler.h"

// === LED Pins ===
const byte profileLEDs[3] = {4, 5, 6};     // LED Grün: Jumper Orange, Jumper Gelb, Jumper Grün
const byte ledHigh = 3;                    // LED Rot: Jumper Braun
const byte ledLow  = 2;                    // LED Rot: Jumper Rot

// === Button Pins ===
const byte btnStartCalibration = 7;       // Jumper Weiß
const byte btnCalibrateNext    = 8;       // Jumper Lila
const byte btnSwitchProfile    = 9;       // Jumper Blau

// === Kalibrierung ===
bool calibrationMode = false;
int calibrationStep = 0;
unsigned long lastBlinkTime = 0;
bool ledState = false;
int blinkCount = 0;
bool waitingForNext = false;

enum CalibrationState {
  CALIB_NONE,
  CALIB_HIGH,
  CALIB_LOW,
  CALIB_DONE
};

CalibrationState calibState = CALIB_NONE;
int calibSensorIdx = 0;

void setupPins() {
  for (byte i = 0; i < 3; i++) pinMode(profileLEDs[i], OUTPUT);
  pinMode(ledHigh, OUTPUT);
  pinMode(ledLow, OUTPUT);
  pinMode(btnStartCalibration, INPUT_PULLUP);
  pinMode(btnCalibrateNext, INPUT_PULLUP);
  pinMode(btnSwitchProfile, INPUT_PULLUP);
}

void updateProfileLEDs() {
  for (byte i = 0; i < 3; i++) {
    digitalWrite(profileLEDs[i], i == currentProfile ? LOW : HIGH); // HIGH-Side Schaltung
  }
}

void startCalibration() {
  calibrationMode = true;
  calibState = CALIB_HIGH;
  calibSensorIdx = 0;
  calibrationStep = 0;
  blinkCount = 0;
  waitingForNext = false;
  lastBlinkTime = millis();
  ledState = false;
  digitalWrite(ledHigh, HIGH);
  digitalWrite(ledLow, HIGH);
}

void cancelCalibration() {
  calibrationMode = false;
  digitalWrite(ledHigh, HIGH);
  digitalWrite(ledLow, HIGH);
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Kalibrierung abgebrochen");
#endif
}

void finishCalibration() {
  for (int i = 0; i < noteCount; i++) {
    thresholds[i] = brightValues[i] - ((brightValues[i] - darkValues[i]) * (thresholdOffsetPercent / 100.0));
    EEPROM.put(eepromStartAddress + currentProfile * profileSize + i * sizeof(int) * 2, brightValues[i]);
    EEPROM.put(eepromStartAddress + currentProfile * profileSize + i * sizeof(int) * 2 + sizeof(int), darkValues[i]);
#ifdef ENABLE_DEBUG_OUTPUT
    Serial.print("Sensor "); Serial.print(i);
    Serial.print(" hell: "); Serial.print(brightValues[i]);
    Serial.print(" dunkel: "); Serial.print(darkValues[i]);
    Serial.print(" threshold: "); Serial.println(thresholds[i]);
#endif
  }
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledHigh, LOW);
    digitalWrite(ledLow, LOW);
    delay(500);
    digitalWrite(ledHigh, HIGH);
    digitalWrite(ledLow, HIGH);
    delay(150);
  }
  calibState = CALIB_NONE;
  calibrationMode = false;
  updateProfileLEDs();
}

void handleCalibrationNext() {
  if (!calibrationMode) return;

  if (calibState == CALIB_HIGH) {
    // Speichere High-Wert
    brightValues[calibSensorIdx] = analogRead(sensorPins[calibSensorIdx]);
    calibState = CALIB_LOW;
    blinkCount = 0;
    waitingForNext = false;
    lastBlinkTime = millis();
    ledState = false;
    digitalWrite(ledHigh, HIGH);
    digitalWrite(ledLow, HIGH);
  } else if (calibState == CALIB_LOW) {
    // Speichere Low-Wert
    darkValues[calibSensorIdx] = analogRead(sensorPins[calibSensorIdx]);
    calibSensorIdx++;
    if (calibSensorIdx >= noteCount) {
      finishCalibration();
      return;
    }
    calibState = CALIB_HIGH;
    blinkCount = 0;
    waitingForNext = false;
    lastBlinkTime = millis();
    ledState = false;
    digitalWrite(ledHigh, HIGH);
    digitalWrite(ledLow, HIGH);
  }
}

void handleCalibrationBlink() {
  if (!calibrationMode || calibState == CALIB_NONE) return;
  if (calibSensorIdx >= noteCount) return;

  unsigned long now = millis();
  int blinkTimes = calibSensorIdx + 1; // Sensor 1 = 1x, Sensor 2 = 2x, ...

  if (waitingForNext) {
    if (now - lastBlinkTime >= 600) { // Pause nach Blink-Zyklus
      blinkCount = 0;
      waitingForNext = false;
    }
    return;
  }

  if (now - lastBlinkTime >= 120) { // Blinkgeschwindigkeit
    ledState = !ledState;
    lastBlinkTime = now;
    if (calibState == CALIB_HIGH) {
      digitalWrite(ledHigh, ledState ? LOW : HIGH);
      digitalWrite(ledLow, HIGH);
    } else if (calibState == CALIB_LOW) {
      digitalWrite(ledLow, ledState ? LOW : HIGH);
      digitalWrite(ledHigh, HIGH);
    }

    if (!ledState) {
      blinkCount++;
      if (blinkCount >= blinkTimes) {
        // Nach X Blinks Pause
        digitalWrite(ledHigh, HIGH);
        digitalWrite(ledLow, HIGH);
        waitingForNext = true;
        lastBlinkTime = now;
      }
    }
  }
}

// --- Button-Status für Entprellung ---
static bool prevStart, prevNext, prevSwitch;
static unsigned long lastStartTime = 0, lastNextTime = 0, lastSwitchTime = 0;
const unsigned long debounceDelay = 50; // 50ms Entprellzeit

// --- Initialisierung der Button-Status ---
void initializeButtons() {
  prevStart  = digitalRead(btnStartCalibration);
  prevNext   = digitalRead(btnCalibrateNext);
  prevSwitch = digitalRead(btnSwitchProfile);
  lastStartTime = lastNextTime = lastSwitchTime = millis();
}

void handleButtons() {
  bool curStart  = digitalRead(btnStartCalibration);
  bool curNext   = digitalRead(btnCalibrateNext);
  bool curSwitch = digitalRead(btnSwitchProfile);
  unsigned long now = millis();

  // Start Calibration Button
  if (curStart != prevStart && (now - lastStartTime) > debounceDelay) {
    lastStartTime = now;
    if (curStart == LOW && prevStart == HIGH) {
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("Start Calibration gedrückt");
#endif
      startCalibration();
    }
  }

  // Calibrate Next Button
  if (curNext != prevNext && (now - lastNextTime) > debounceDelay) {
    lastNextTime = now;
    if (curNext == LOW && prevNext == HIGH) {
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("Calibrate Next gedrückt");
#endif
      handleCalibrationNext();
    }
  }

  // Switch Profile Button
  if (curSwitch != prevSwitch && (now - lastSwitchTime) > debounceDelay) {
    lastSwitchTime = now;
    if (curSwitch == LOW && prevSwitch == HIGH) {
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("Profilwechsel gedrückt");
#endif
      if (calibrationMode) {
        cancelCalibration();
        // Profil NICHT wechseln!
      } else {
        currentProfile = (currentProfile + 1) % 3;
        loadProfile(currentProfile);
        updateProfileLEDs();
      }
    }
  }

  prevStart  = curStart;
  prevNext   = curNext;
  prevSwitch = curSwitch;
}

// --- Rote LEDs außerhalb der Kalibrierung immer aus ---
void updateCalibrationLEDs() {
  if (!calibrationMode) {
    digitalWrite(ledHigh, HIGH);
    digitalWrite(ledLow, HIGH);
  }
}

#endif // BUTTONS_AND_LEDS_H
