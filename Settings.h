#ifndef SETTINGS_H
#define SETTINGS_H

#include "NoteLayouts.h"

#include <wiring.h>

struct Settings {
  int noteLayout = NOTELAYOUTTYPE_MANOURY;
  int forceBellows = 0; // 1 means use opening. -1 means use closing. 0 means use the pressure sensor
  float pressureGain = 0.5f;

  uint32_t debounceTime = 10;  // milliseconds

  int midiChannelLeft = 1;
  int midiChannelRight = 2;

  // percentages between -100 and 100
  int panLeft = -50;
  int panRight = 50;

  // Shape the pressure response using attack params. 
  float attack25 = 0.25;  // output when pressure = 25%
  float attack50 = 0.50;  // output when pressure = 50%
  float attack75 = 0.75;  // output when pressure = 75%

  // Things below here are updated automatically
  byte midiMin = 0;
  byte midiMax = 127;

  // Call this to limit the range of midi notes we traverse after changing the layout
  void updateMIDIRange();
};

extern Settings settings;

#endif
