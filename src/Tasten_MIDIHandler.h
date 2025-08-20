// File: MIDIHandler.h
#ifndef Tasten_MIDI_HANDLER_H
#define Tasten_MIDI_HANDLER_H

#include <Arduino.h>
#include <SoftwareSerial.h>

// Funktionsprototypen
void handleSensors();
void noteOn(byte channel, byte pitch, byte velocity);
void noteOff(byte channel, byte pitch, byte velocity);
void sendAftertouch(byte channel, byte pitch, byte pressure);
void sendMIDI(byte status, byte data1, byte data2);
void debugPrint(const char* label, byte ch, byte note, byte val);

//--------------------------------------

// === MIDI-Konfiguration ===
const byte midiTxPin = 10;
SoftwareSerial midiSerial(midiTxPin, 255);
const byte midiChannel = 1;
const byte noteCount = 5;
const byte noteNumbers[noteCount] = {60, 62, 64, 65, 67};  // C4–G4
const byte sensorPins[noteCount] = {A0, A1, A2, A3, A4};
int thresholds[noteCount];
int brightValues[noteCount];
int darkValues[noteCount];
const int hysteresis = 20;
const float thresholdOffsetPercent = 5.0;

// === EEPROM ===
const int eepromStartAddress = 0;
const int profileSize = noteCount * sizeof(int) * 2;
byte currentProfile = 0;

// === Status ===
bool noteIsOn[noteCount] = {false};
int aftertouch[noteCount] = {0};

void loadProfile(byte profile) {
  for (int i = 0; i < noteCount; i++) {
    EEPROM.get(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2, brightValues[i]);
    EEPROM.get(eepromStartAddress + profile * profileSize + i * sizeof(int) * 2 + sizeof(int), darkValues[i]);
    thresholds[i] = brightValues[i] - ((brightValues[i] - darkValues[i]) * (thresholdOffsetPercent / 100.0));
  }
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Profil "); Serial.print(profile + 1); Serial.println(" geladen");
#endif
}

void handleSensors() {
  for (byte i = 0; i < noteCount; i++) {
    int value = analogRead(sensorPins[i]);
    int velocity = map(value, 1023, 0, 0, 127);
    velocity = constrain(velocity, 0, 127);

    if (value < thresholds[i] - hysteresis && !noteIsOn[i]) {
      noteOn(midiChannel, noteNumbers[i], velocity);
      noteIsOn[i] = true;
      aftertouch[i] = velocity;
    } else if (value > thresholds[i] + hysteresis && noteIsOn[i]) {
      noteOff(midiChannel, noteNumbers[i], 0);
      noteIsOn[i] = false;
    } else if (noteIsOn[i]) {
      int newAftertouch = velocity;
      if (abs(newAftertouch - aftertouch[i]) > 4) {
        sendAftertouch(midiChannel, noteNumbers[i], newAftertouch);
        aftertouch[i] = newAftertouch;
      }
    }
    delay(5);
  }
}

void noteOn(byte channel, byte pitch, byte velocity) {
  sendMIDI(0x90 | (channel - 1), pitch, velocity);
  debugPrint("Note ON", channel, pitch, velocity);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  sendMIDI(0x80 | (channel - 1), pitch, velocity);
  debugPrint("Note OFF", channel, pitch, velocity);
}

void sendAftertouch(byte channel, byte pitch, byte pressure) {
  sendMIDI(0xA0 | (channel - 1), pitch, pressure);
  debugPrint("Aftertouch", channel, pitch, pressure);
}

void sendMIDI(byte status, byte data1, byte data2) {
  midiSerial.write(status);
  midiSerial.write(data1);
  midiSerial.write((uint8_t)data2);
}

void debugPrint(const char* label, byte ch, byte note, byte val) {
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print(label);
  Serial.print(" | Ch: ");
  Serial.print(ch);
  Serial.print(" | Note: ");
  Serial.print(note);
  Serial.print(" | Value: ");
  Serial.println(val);
#endif
}

#endif // MIDI_HANDLER_H
