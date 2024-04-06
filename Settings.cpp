#include "Settings.h"
#include "NoteLayouts.h"
#include "PinInputs.h"
#include "State.h"

#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>

#include <algorithm>

Settings settings;

//====================================================================================================
void Settings::updateMIDIRange() {
  midiMin = 127;
  midiMax = 0;
  if (bigState.noteLayoutLeftOpen) {
    for (int i = 0; i != PinInputs::keyCountLeft; ++i) {
      midiMax = std::max(bigState.noteLayoutLeftOpen[i], midiMax);
      midiMin = std::min(bigState.noteLayoutLeftOpen[i], midiMin);
      midiMax = std::max(bigState.noteLayoutLeftClose[i], midiMax);
      midiMin = std::min(bigState.noteLayoutLeftClose[i], midiMin);
    }
  }
  if (bigState.noteLayoutRightOpen) {
    for (int i = 0; i != PinInputs::keyCountRight; ++i) {
      midiMax = std::max(bigState.noteLayoutRightOpen[i], midiMax);
      midiMin = std::min(bigState.noteLayoutRightOpen[i], midiMin);
      midiMax = std::max(bigState.noteLayoutRightClose[i], midiMax);
      midiMin = std::min(bigState.noteLayoutRightClose[i], midiMin);
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
  WRITE_SETTING(pressureGain);
  WRITE_SETTING(debounceTime);
  WRITE_SETTING(midiChannelLeft);
  WRITE_SETTING(midiChannelRight);
  WRITE_SETTING(panLeft);
  WRITE_SETTING(panRight);
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

#define READ_SETTING(x) x = doc[#x];

  READ_SETTING(slot);
  READ_SETTING(noteLayout);
  READ_SETTING(noteLayout);
  READ_SETTING(forceBellows);
  READ_SETTING(pressureGain);
  READ_SETTING(debounceTime);
  READ_SETTING(midiChannelLeft);
  READ_SETTING(midiChannelRight);
  READ_SETTING(panLeft);
  READ_SETTING(panRight);
  READ_SETTING(attack25);
  READ_SETTING(attack50);
  READ_SETTING(attack75);
  READ_SETTING(midiMin);
  READ_SETTING(midiMax);

  file.close();
  Serial.printf("Settings read from %s\n", filename);
  return true;
}
