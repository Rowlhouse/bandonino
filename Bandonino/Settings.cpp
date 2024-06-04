#include "Settings.h"
#include "NoteLayouts.h"
#include "PinInputs.h"
#include "State.h"

// https://arduinojson.org/
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>

#include <algorithm>

Settings gSettings;

const char* gExpressionNames[] = {
  "Volume", "Velocity"
};

const char* gNoteDisplayNames[] = {
  "Stacked", "Placed"
};

//====================================================================================================
void Settings::updateMIDIRange() {
  midiMin = 127;
  midiMax = 0;
  if (!gBigState.mNoteLayout.mLeftOpen)
    return;
  for (int j = 0; j != 2; ++j) {
    for (int i = 0; i != PinInputs::keyCounts[j]; ++i) {
      midiMax = std::max(gBigState.mNoteLayout.open(j)[i], midiMax);
      midiMin = std::min(gBigState.mNoteLayout.open(j)[i], midiMin);
      midiMax = std::max(gBigState.mNoteLayout.close(j)[i], midiMax);
      midiMin = std::min(gBigState.mNoteLayout.close(j)[i], midiMin);
    }
  }
}

//====================================================================================================
bool initCard() {
  Serial.print("Initializing SD card...");
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("initialization failed!");
    return false;
  }
  Serial.println("initialization done.");
  return true;
}

//====================================================================================================
// Prints the content of a file to the Serial
void printFile(const char* filename) {
  // Open file for reading
  File file = SD.open(filename);
  if (!file) {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

//====================================================================================================
bool Settings::writeToCard(const char* filename) {
  if (!initCard())
    return false;

  SD.remove(filename);
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.printf("Failed to create file %f\n", filename);
    return false;
  }

  JsonDocument doc;
#define WRITE_SETTING(x) doc[#x] = x;
  WRITE_SETTING(slot);
  WRITE_SETTING(noteLayout);
  WRITE_SETTING(forceBellows);
  WRITE_SETTING(zeroLoadReading);
  WRITE_SETTING(expressions[LEFT]);
  WRITE_SETTING(expressions[RIGHT]);
  WRITE_SETTING(maxVelocity[LEFT]);
  WRITE_SETTING(maxVelocity[RIGHT]);
  WRITE_SETTING(noteOffVelocity[LEFT]);
  WRITE_SETTING(noteOffVelocity[RIGHT]);
  WRITE_SETTING(octave[LEFT]);
  WRITE_SETTING(octave[RIGHT]);
  WRITE_SETTING(transpose);
  WRITE_SETTING(pressureGain);
  WRITE_SETTING(debounceTime);
  WRITE_SETTING(midiChannels[LEFT]);
  WRITE_SETTING(midiChannels[RIGHT]);
  WRITE_SETTING(midiInstruments[LEFT]);
  WRITE_SETTING(midiInstruments[RIGHT]);
  WRITE_SETTING(metronomeEnabled);
  WRITE_SETTING(metronomeBeatsPerMinute);
  WRITE_SETTING(metronomeBeatsPerBar);
  WRITE_SETTING(metronomeVolume);
  WRITE_SETTING(metronomeMidiNotePrimary);
  WRITE_SETTING(metronomeMidiNoteSecondary);
  WRITE_SETTING(metronomeMidiChannel);
  WRITE_SETTING(metronomeMidiInstrument);
  WRITE_SETTING(metronomeLED);
  WRITE_SETTING(stereo);
  WRITE_SETTING(balance);
  WRITE_SETTING(showFPS);
  WRITE_SETTING(menuBrightness);
  WRITE_SETTING(noteDisplay);
  WRITE_SETTING(accidentalPreference);
  WRITE_SETTING(accidentalKey);
  WRITE_SETTING(menuPageIndex);
  WRITE_SETTING(menuDisplayEnabled);
  WRITE_SETTING(attack25);
  WRITE_SETTING(attack50);
  WRITE_SETTING(attack75);
  WRITE_SETTING(midiMin);
  WRITE_SETTING(midiMax);

  if (serializeJson(doc, file) == 0) {
    Serial.printf("Failed to serialize JSON to file %s\n", filename);
    file.close();
    return false;
  }

  file.close();
  Serial.printf("Settings written to %s\n", filename);
  return true;
}

//====================================================================================================
bool Settings::readFromCard(const char* filename) {
  if (!initCard())
    return false;
  File file = SD.open(filename, FILE_READ);
  if (!file) {
    Serial.printf("Failed to open file %s for reading\n", filename);
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.printf("Failed to read/deserialize file %s\n", filename);
    file.close();
    return false;
  }

#define READ_SETTING(x) x = doc[#x] | x

  READ_SETTING(slot);
  READ_SETTING(noteLayout);
  READ_SETTING(forceBellows);
  READ_SETTING(zeroLoadReading);
  READ_SETTING(expressions[LEFT]);
  READ_SETTING(expressions[RIGHT]);
  READ_SETTING(maxVelocity[LEFT]);
  READ_SETTING(maxVelocity[RIGHT]);
  READ_SETTING(noteOffVelocity[LEFT]);
  READ_SETTING(noteOffVelocity[RIGHT]);
  READ_SETTING(octave[LEFT]);
  READ_SETTING(octave[RIGHT]);
  READ_SETTING(transpose);
  READ_SETTING(pressureGain);
  READ_SETTING(debounceTime);
  READ_SETTING(midiChannels[LEFT]);
  READ_SETTING(midiChannels[RIGHT]);
  READ_SETTING(midiInstruments[LEFT]);
  READ_SETTING(midiInstruments[RIGHT]);
  // READ_SETTING(metronomeEnabled); Never automatically turn it on
  READ_SETTING(metronomeBeatsPerMinute);
  READ_SETTING(metronomeBeatsPerBar);
  READ_SETTING(metronomeVolume);
  READ_SETTING(metronomeMidiNotePrimary);
  READ_SETTING(metronomeMidiNoteSecondary);
  READ_SETTING(metronomeMidiChannel);
  READ_SETTING(metronomeMidiInstrument);
  READ_SETTING(metronomeLED);
  READ_SETTING(stereo);
  READ_SETTING(balance);
  READ_SETTING(showFPS);
  READ_SETTING(menuBrightness);
  READ_SETTING(noteDisplay);
  READ_SETTING(accidentalPreference);
  READ_SETTING(accidentalKey);
  READ_SETTING(menuPageIndex);
  READ_SETTING(menuDisplayEnabled);
  READ_SETTING(attack25);
  READ_SETTING(attack50);
  READ_SETTING(attack75);
  READ_SETTING(midiMin);
  READ_SETTING(midiMax);

  // Avoid problems reading bad data!
  slot = std::clamp(slot, 0, 10);
  menuBrightness = std::clamp(menuBrightness, 4, 16);

  file.close();
  Serial.printf("Settings read from %s\n", filename);
  return true;
}

//====================================================================================================
void Settings::reset() {
  auto origSettings = *this;
  *this = Settings();
  zeroLoadReading = origSettings.zeroLoadReading;
}
