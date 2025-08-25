// Instrumentenwahl über Build-Flag
//#define ENABLE_DEBUG_OUTPUT

#include <Arduino.h>

#ifdef INSTRUMENT_TASTEN
#include "Tasten_Control.h"
#include "Tasten_MIDIHandler.h"
#endif

#ifdef INSTRUMENT_PERCUSSION
#include "Percussion_Control.h"
#include "Percussion_MIDIHandler.h"
#endif

#ifdef INSTRUMENT_BLASROHR
#include "Blasrohr_Control.h"
#include "Blasrohr_MIDIHandler.h"
#endif

void setup() {
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.begin(115200);
#endif
#ifdef INSTRUMENT_TASTEN
  setupMIDI();
  setupPins();
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Starting up MIDI profile "); Serial.println(currentProfile + 1);
#endif
  loadProfile(currentProfile);
  updateProfileLEDs();
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Setup Abgeschlossen.");
#endif
#endif

#ifdef INSTRUMENT_PERCUSSION
  // TODO: MIDI/Serial und Pins für Percussion initialisieren
  setupMIDI();
  setupPercussion();
  #ifdef ENABLE_DEBUG_OUTPUT
    Serial.print("Starting up MIDI profile "); Serial.println(currentProfile + 1);
  #endif
  loadPercussionProfile(currentProfile);
  #ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("Setup Abgeschlossen.");
  #endif
#endif

#ifdef INSTRUMENT_BLASROHR
  // TODO: MIDI/Serial und Pins für Blasrohr initialisieren
  //midiSerial.begin(31250);
  setupBlasrohrPins();
  loadBlasrohrProfile(currentBlasrohrProfile);
#endif
}

void loop() {
#ifdef INSTRUMENT_TASTEN
  handleButtons();
  if (calibrationMode) {
    handleCalibrationBlink();
  } else {
    handleSensors();
    updateCalibrationLEDs();
  }
#endif

#ifdef INSTRUMENT_PERCUSSION
  handlePercussionButtons();
  if (mode == MODE_CALIBRATE) {
    handlePercussionCalibration();
  } else {
    handlePercussionSensors();
  }
  updatePercussionScreen();
#endif

#ifdef INSTRUMENT_BLASROHR
  handleBlasrohrButtons();
  if (blasrohrCalibrationMode) {
    handleBlasrohrCalibration();
  } else {
    handleBlasrohrSensors();
    updateBlasrohrScreen();
  }
#endif
}