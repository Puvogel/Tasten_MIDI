// File: Percussion_Control.h
#ifndef PERCUSSION_CONTROL_H
#define PERCUSSION_CONTROL_H

#include <Arduino.h>
#include <EEPROM.h>
#include <Adafruit_SSD1306.h>
//#include <SoftwareSerial.h>
#include "EEPROM_Handler.h"

// === Pinout ===
// SSD1306 OLED
#define OLED_MOSI   11
#define OLED_CLK    12
#define OLED_DC     8
#define OLED_RST    9
#define OLED_CS     10

// Buttons
#define BTN_SWITCH_PROFILE 3
#define BTN_SWITCH_MODE    4

// Sensor Pins
extern const byte padCount;
extern const byte sensorPins[6];

// === Display ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

const byte padCount = 6;
const byte sensorPins[6] = {A0, A1, A2, A3, A4, A5};

// === Profile & Calibration ===
const int eepromStartAddress = 0;
const int profileSize = padCount * sizeof(int) * 2;
byte currentProfile = 0;

// Sensorwerte
int restValues[padCount];      // Ruhewerte
int peakValues[padCount];      // Maximale Ausschläge

// Kalibrierung
unsigned long calibrationStartTime = 0;
bool calibrationAborted = false;
bool restCaptured = false;
int lastHitSensor = -1;
int peakPositive[padCount];
int peakNegative[padCount];

// Modus
enum PercussionMode { MODE_PLAY, MODE_CALIBRATE };
PercussionMode mode = MODE_PLAY;

// --- Button-Status für Entprellung ---
static bool prevMode, prevProfile;
static unsigned long lastStartTime = 0, lastNextTime = 0, lastSwitchTime = 0;
const unsigned long debounceDelay = 50; // 50ms Entprellzeit

// Forward declaration for initializeButtons
void initializeButtons();

// === Setup Pins ===
void setupPercussion() {
  pinMode(BTN_SWITCH_PROFILE, INPUT_PULLUP);
  pinMode(BTN_SWITCH_MODE, INPUT_PULLUP);
  for (byte i = 0; i < padCount; i++) pinMode(sensorPins[i], INPUT);
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Initialisiere SSD1306 Display...");
#endif
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
#ifdef ENABLE_DEBUG_OUTPUT
    Serial.println("Display-Initialisierung fehlgeschlagen!");
#endif
    while (true) { delay(1000); } // Stoppe das Programm
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Display erfolgreich initialisiert.");
#endif
  initializeButtons();
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Buttons erfolgreich initialisiert.");
#endif
}

void initializeButtons() {
  prevMode  = digitalRead(BTN_SWITCH_MODE);
  prevProfile = digitalRead(BTN_SWITCH_PROFILE);
  lastStartTime = lastNextTime = lastSwitchTime = millis();
}

// === Display Update ===
void updatePercussionScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("M: ");
  display.print(mode == MODE_PLAY ? "Play" : "Calibrate");
  display.print(", P: ");
  display.print(currentProfile + 1);
  display.setCursor(0, 10);
  if (mode == MODE_PLAY) {
    // Zeige z.B. letzte Note und Velocity (optional: globale Variable für letzte MIDI-Ausgabe)
    extern byte lastPercNote, lastPercVelocity;
    char buf[32];
    snprintf(buf, sizeof(buf), "Note %d Vel %d", lastPercNote, lastPercVelocity);
    display.print(buf);
#ifdef ENABLE_DEBUG_OUTPUT
    //Serial.println(buf);
#endif
  } else if (mode == MODE_CALIBRATE) {
    if (calibrationAborted) {
  display.println("Calibration Aborted");
  calibrationAborted = false;
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Calibration Aborted");
#endif
    } else if (!restCaptured) {
  display.print("Ruhewert wird erfasst...");
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Ruhewert wird erfasst...");
#endif
    } else if (lastHitSensor >= 0) {
  char buf[32];
  snprintf(buf, sizeof(buf), "S%d R:%d P:%d", lastHitSensor+1, restValues[lastHitSensor], abs(peakPositive[lastHitSensor] - restValues[lastHitSensor]) > abs(peakNegative[lastHitSensor] - restValues[lastHitSensor]) ? peakPositive[lastHitSensor] : peakNegative[lastHitSensor]);
  display.print(buf);
#ifdef ENABLE_DEBUG_OUTPUT
  //Serial.println(buf);
#endif
    } else {
  display.print("Kalibrierung aktiv");
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Kalibrierung aktiv");
#endif
    }
  }
  display.display();
}

// === EEPROM Profile laden/speichern ===
void loadPercussionProfile(byte profile) {
  loadPercussionProfile(profile, restValues, peakValues, padCount, eepromStartAddress, profileSize);
}
void savePercussionProfile(byte profile) {
  savePercussionProfile(profile, restValues, peakValues, padCount, eepromStartAddress, profileSize);
}

// === Button Handling ===
void handlePercussionButtons() {
  bool curStart  = digitalRead(BTN_SWITCH_MODE);
  bool curSwitch = digitalRead(BTN_SWITCH_PROFILE);
  unsigned long now = millis();

  // Switch Mode Button
  if (curStart != prevMode && (now - lastStartTime) > debounceDelay) {
    lastStartTime = now;
    if (curStart == LOW && prevMode == HIGH) {
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("Switch Mode gedrückt");
#endif
      if (mode == MODE_PLAY) {
        mode = MODE_CALIBRATE;
        restCaptured = false;
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Modus gewechselt: Calibrate");
#endif
      } else if (mode == MODE_CALIBRATE) {
  savePercussionProfile(currentProfile);
  mode = MODE_PLAY;
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Modus gewechselt: Play und Werte gespeichert");
#endif
      } else {
  #ifdef ENABLE_DEBUG_OUTPUT
  Serial.println("Unbekannter Modus");
  delay(1000);
  #endif
      }
    }
  }

  // Switch Profile Button
  if (curSwitch != prevProfile && (now - lastSwitchTime) > debounceDelay) {
    lastSwitchTime = now;
    if (curSwitch == LOW && prevProfile == HIGH) {
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("Profilwechsel gedrückt");
#endif
      if (mode == MODE_CALIBRATE) {
      calibrationAborted = true;
      mode = MODE_PLAY;
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("Calibration Aborted");
#endif
    } else if (mode == MODE_PLAY) {
      currentProfile = (currentProfile + 1) % 3;
      loadPercussionProfile(currentProfile);
#ifdef ENABLE_DEBUG_OUTPUT
      Serial.println("Profil gewechselt und geladen");
#endif
        }
    }
  }
  prevMode  = curStart;
  prevProfile = curSwitch;
}

void handlePercussionCalibration() {
  if (mode == MODE_PLAY) return;
  // Ruhewert für alle Sensoren gleichzeitig nach 1 Sekunde erfassen
  if (!restCaptured && millis() - calibrationStartTime >= 1000) {
    updatePercussionScreen();         // Um den aktuellen Kalibirierungs-Status anzuzeigen
    for (int i = 0; i < padCount; i++) {
      long sum = 0;
      for (int j = 0; j < 200; j++) { // 200 Messungen in 1 Sekunde
        sum += analogRead(sensorPins[i]);
        delay(5);
      }
      restValues[i] = sum / 200;
      peakPositive[i] = restValues[i];
      peakNegative[i] = restValues[i];
    }
    restCaptured = true;
  // Anzeige erfolgt über updatePercussionScreen()
  }
  // Peaks fortlaufend erfassen, solange Kalibrierung aktiv
  if (restCaptured) {
    updatePercussionScreen();       // Um den Kalibrierungs-Status anzuzeigen
    for (int i = 0; i < padCount; i++) {
      int val = analogRead(sensorPins[i]);
      if (val - restValues[i] > peakPositive[i] - restValues[i]) {
        peakPositive[i] = val;
        lastHitSensor = i;
      }
      if (val - restValues[i] < peakNegative[i] - restValues[i]) {
        peakNegative[i] = val;
        lastHitSensor = i;
      }
    }
    // Display: Zeige Ruhewert und Peak für zuletzt ausgelösten Sensor
  // Anzeige erfolgt über updatePercussionScreen()
  }
}

#endif // PERCUSSION_CONTROL_H
