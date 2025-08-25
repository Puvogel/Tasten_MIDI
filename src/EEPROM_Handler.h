// File: EEPROM_Handler.h
#ifndef EEPROM_HANDLER_H
#define EEPROM_HANDLER_H

#include <Arduino.h>
#include <EEPROM.h>

// Lädt die Profile-Werte (high/low) für einen Sensorbereich
void loadProfile(byte profile, int* highValues, int* lowValues, int* thresholds, byte noteCount, int eepromStartAddress, int profileSize, float thresholdOffsetPercent) {
  for (int i = 0; i < noteCount; i++) {
    EEPROM.get(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2, highValues[i]);
    EEPROM.get(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2 + sizeof(int), lowValues[i]);
    thresholds[i] = highValues[i] - ((highValues[i] - lowValues[i]) * (thresholdOffsetPercent / 100.0));
  }
}

// Lädt Ruhewert und Maximalausschlag für alle Pads
void loadPercussionProfile(byte profile, int* restValues, int* peakValues, byte padCount, int eepromStartAddress, int profileSize) {
  for (int i = 0; i < padCount; i++) {
    EEPROM.get(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2, restValues[i]);
    EEPROM.get(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2 + sizeof(int), peakValues[i]);
  }
}

// Speichert die Profile-Werte (high/low) für einen Sensorbereich
void saveProfile(byte profile, int* highValues, int* lowValues, byte noteCount, int eepromStartAddress, int profileSize) {
  for (int i = 0; i < noteCount; i++) {
    EEPROM.put(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2, highValues[i]);
    EEPROM.put(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2 + sizeof(int), lowValues[i]);
  }
}

// Speichert Ruhewert und Maximalausschlag für alle Pads
void savePercussionProfile(byte profile, int* restValues, int* peakValues, byte padCount, int eepromStartAddress, int profileSize) {
  for (int i = 0; i < padCount; i++) {
    EEPROM.put(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2, restValues[i]);
    EEPROM.put(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2 + sizeof(int), peakValues[i]);
  }
}

#endif // EEPROM_HANDLER_H
