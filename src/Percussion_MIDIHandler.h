// Test-Flag für MIDI-Testfunktion
#define ENABLE_MIDI_TEST 0

// File: Percussion_MIDIHandler.h
#ifndef PERCUSSION_MIDI_HANDLER_H
#define PERCUSSION_MIDI_HANDLER_H

#include <Arduino.h>
//#include <SoftwareSerial.h>


// Für Display-Ausgabe
byte lastPercNote = 0;
byte lastPercVelocity = 0;

// MIDI-Konfiguration
//#define MIDI_TX_PIN 6

//SoftwareSerial midiSerial(MIDI_TX_PIN, 255);

const byte midiChannel = 10; // Standard Drum Channel
// Feste Größe für Arrays
#define PERC_PAD_COUNT 6
extern const byte sensorPins[PERC_PAD_COUNT];
extern int restValues[PERC_PAD_COUNT];
extern int peakValues[PERC_PAD_COUNT];
const byte padNotes[PERC_PAD_COUNT] = {36, 38, 40, 41, 43, 45}; // Beispiel: Bass, Snare, Tom1, Tom2, Tom3, HiHat

// Status
bool padIsOn[PERC_PAD_COUNT] = {false};

// Funktionsprototypen
void noteOn(byte channel, byte pitch, byte velocity);
void noteOff(byte channel, byte pitch, byte velocity);
void sendMIDI(byte status, byte data1, byte data2);

void setupMIDI() {
  Serial.begin(31250);
}

// Sensor auslesen und MIDI senden
void handlePercussionSensors() {
  static unsigned long lastScanTime = 0;
  static int peakValue[PERC_PAD_COUNT] = {0};
  static int peakVelocity[PERC_PAD_COUNT] = {0};
  static bool peakDetected[PERC_PAD_COUNT] = {false};
  unsigned long now = millis();

  // Messzeitraum (z.B. 20ms)
  if (now - lastScanTime > 20) {
    // Sende für jeden Pad den höchsten Peak als MIDI
    for (byte i = 0; i < PERC_PAD_COUNT; i++) {
      if (peakDetected[i]) {
        noteOn(midiChannel, padNotes[i], peakVelocity[i]);
        lastPercNote = padNotes[i];
        lastPercVelocity = peakVelocity[i];
        padIsOn[i] = true;
#ifdef ENABLE_DEBUG_OUTPUT
        Serial.print("Peak MIDI: Pad "); Serial.print(i); Serial.print(" Note "); Serial.print(padNotes[i]);
        Serial.print(" Velocity "); Serial.println(peakVelocity[i]);
#endif
      }
      peakValue[i] = 0;
      peakVelocity[i] = 0;
      peakDetected[i] = false;
    }
    lastScanTime = now;
  }

  // Erfasse Peaks innerhalb des Zeitraums
  for (byte i = 0; i < PERC_PAD_COUNT; i++) {
    int value = analogRead(sensorPins[i]);
    int rest = restValues[i];
    int peak = peakValues[i];
    int velocity = 0;
    if (peak != rest) {
      velocity = map(abs(value - rest), 0, abs(peak - rest), 0, 127);
      velocity = constrain(velocity, 0, 127);
    }
    if (abs(value - rest) > 20) {
      if (!peakDetected[i] || abs(value - rest) > abs(peakValue[i] - rest)) {
        peakValue[i] = value;
        peakVelocity[i] = velocity;
        peakDetected[i] = true;
      }
    }
    // Optional: NoteOff nach kurzer Zeit
    if (padIsOn[i] && abs(value - rest) < 10) {
      noteOff(midiChannel, padNotes[i], 0);
      padIsOn[i] = false;
    }
  }
}

void noteOn(byte channel, byte pitch, byte velocity) {
  sendMIDI(0x90 | (channel - 1), pitch, velocity);
}
void noteOff(byte channel, byte pitch, byte velocity) {
  sendMIDI(0x80 | (channel - 1), pitch, velocity);
}
void sendMIDI(byte status, byte data1, byte data2) {
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("MIDI gesendet: ");
  Serial.print("Status: 0x"); Serial.print(status, HEX);
  Serial.print(" Data1: "); Serial.print(data1);
  Serial.print(" Data2: "); Serial.println(data2);
#endif
  Serial.write(status);
  Serial.write(data1);
  Serial.write((uint8_t)data2);
}

#endif // PERCUSSION_MIDI_HANDLER_H
