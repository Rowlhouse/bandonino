#ifndef SETTINGS_H
#define SETTINGS_H

#include "NoteLayouts.h"
#include "NoteNames.h"

#include <stdint.h>
#include <limits.h>

enum Expression {
  EXPRESSION_VOLUME,
  EXPRESSION_VELOCITY,
  EXPRESSION_NUM
};
extern const char* gExpressionNames[];

enum NoteDisplay {
  NOTE_DISPLAY_STACKED,
  NOTE_DISPLAY_PLACED,
  NOTE_DISPLAY_NUM
};
extern const char* gNoteDisplayNames[];

struct Settings {
  int slot;  // settings slot - e.g. 0 to 9
  int noteLayout = NOTELAYOUTTYPE_MANOURY2;
  int forceBellows = 0;    // 1 means use opening. -1 means use closing. 0 means use the pressure sensor
  long zeroLoadReading = LONG_MAX ; // Reasonable LONG_MAX means to measure
  int  zeroLoadOffset = 0; // Will be applied and then immediately zeroed
  int pressureGain = 100;  // Treat as percentage - but it can go above 100
  int expressions[2] = { EXPRESSION_VOLUME, EXPRESSION_VOLUME };
  int maxVelocity[2] = { 126, 126 };
  int noteOffVelocity[2] = { 64, 64 };
  int octave[2] = { 0, 0 };  // up/down in octaves, per side
  int transpose = 0;         // in semitones

  int debounceTime = 0;  // milliseconds. Turns out not to be very helpful

  int midiChannels[2] = { 1, 2 };
  int midiInstruments[2] = { -1, -1 };  // -1 means don't send - let the playback system decide

  bool metronomeEnabled = false;
  int metronomeBeatsPerMinute = 100;
  int metronomeBeatsPerBar = 4;
  int metronomeVolume = 50;
  int metronomeMidiNotePrimary = 60;
  int metronomeMidiNoteSecondary = 80;
  int metronomeMidiChannel = 3;
  int metronomeMidiInstrument = 115;  // Appears to be woodblock
  bool metronomeLED = true;

  // percentages between -100 and 100
  // int pans[2] = { -25, 25 };

  // 0 is mono. 100 is full left/right. -100 is right/left
  int stereo = 50;
  // 0 is equal treble/bass. 100 is treble only. -100 is bass only
  int balance = 0;

  // Percentages between 0 and 100
  // int levels[2] = { 100, 100 };

  int showFPS = 0;
  int menuBrightness = 12;  // 0 to 0xf
  int noteDisplay = NOTE_DISPLAY_STACKED;
  int accidentalPreference = ACCIDENTAL_PREFERENCE_KEY;
  int accidentalKey = KEY_OFFSET;

  int menuPageIndex = 2;
  bool menuDisplayEnabled = true;

  // Shape the pressure response using attack params. As percentages
  int deadzone = 10;  // pressure is rescaled to remove this %
  int attack25 = 25;  // output when pressure = 25%
  int attack50 = 55;  // output when pressure = 50%
  int attack75 = 75;  // output when pressure = 75%

  // Things below here are updated automatically
  uint8_t midiMin = 0;
  uint8_t midiMax = 127;

  // Call this to limit the range of midi notes we traverse after changing the layout
  void updateMIDIRange();

  // Sets everything to defaults EXCEPT the zero load reading is preserved
  void reset();

  bool writeToCard(const char* filename = "gSettings.json");
  bool readFromCard(const char* filename = "gSettings.json");
};

extern Settings gSettings;

#endif
