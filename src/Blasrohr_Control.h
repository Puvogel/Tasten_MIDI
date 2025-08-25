// File: Blasrohr_Control.h
#ifndef BLASROHR_CONTROL_H
#define BLASROHR_CONTROL_H

#include <Arduino.h>

// Stub-Funktionen für Blasrohr
void setupBlasrohrPins() {}
void loadBlasrohrProfile(byte profile) {}
void updateBlasrohrLEDs() {}
void handleBlasrohrButtons() {}
bool blasrohrCalibrationMode = false;
void handleBlasrohrCalibration() {}
void updateBlasrohrScreen() {}
byte currentBlasrohrProfile = 0;

#endif // BLASROHR_CONTROL_H
