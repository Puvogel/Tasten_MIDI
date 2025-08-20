#define ENABLE_DEBUG_OUTPUT

#include <Arduino.h>
#include "Tasten_ButtonsAndLEDs.h"
#include "Tasten_MIDIHandler.h"

void setup() {
  Serial.begin(115200);
  midiSerial.begin(31250);

  setupPins();
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Starting up MIDI profile "); Serial.println(currentProfile + 1);
#endif
  loadProfile(currentProfile);
  updateProfileLEDs();
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Setup Abgeschlossen.");
#endif
}

void loop() {
  handleButtons();
  if (calibrationMode) {
    handleCalibrationBlink();
  } else {
    handleSensors();
    updateCalibrationLEDs(); // <-- Rote LEDs außerhalb der Kalibrierung aus
  }
}