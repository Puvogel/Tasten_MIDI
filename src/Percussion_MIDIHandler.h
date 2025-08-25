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
  for (byte i = 0; i < PERC_PAD_COUNT; i++) {
    int value = analogRead(sensorPins[i]);
    int rest = restValues[i];
    int peak = peakValues[i];
    int velocity = 0;
    // Normierung: Wert zwischen Ruhewert und Peak auf MIDI-Range
    if (peak != rest) {
      velocity = map(abs(value - rest), 0, abs(peak - rest), 0, 127);
      velocity = constrain(velocity, 0, 127);
    }
    // Schlag erkennen: Abweichung vom Ruhewert > Schwellwert
    if (abs(value - rest) > 50 && !padIsOn[i]) {
      noteOn(midiChannel, padNotes[i], velocity);
      lastPercNote = padNotes[i];
      lastPercVelocity = velocity;
      padIsOn[i] = true;
    } else if (abs(value - rest) < 10 && padIsOn[i]) {
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
