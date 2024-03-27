#ifndef SETTINGS_H
#define SETTINGS_H

#include "NoteLayouts.h"

#include <wiring.h>

struct Settings {
  bool useBellows = true;
  const int pressureGain = 8;

  const uint32_t debounceTime = 10;  // milliseconds

  const int midiChannelLeft = 1;
  const int midiChannelRight = 2;

  // percentages between -100 and 100
  int panLeft = -50;
  int panRight = 50;

  // The actual note layout
  const byte* noteLayoutLeftOpen = nullptr;
  const byte* noteLayoutRightOpen = nullptr;
  const byte* noteLayoutLeftClose = nullptr;
  const byte* noteLayoutRightClose = nullptr;

  byte midiMin = 0;
  byte midiMax = 127;

  // Shape the pressure response using attack params. Each is interpreted as a fraction out of 32 (i.e. 16 is 0.5)
  int attack1 = 56;  // output when pressure = 32
  int attack2 = 80;  // output when pressure = 64
  int attack3 = 104; // output when pressure = 96
  // 127 will turn into 127

  // Call this to limit the range of midi notes we traverse after changing the layout
  void updateMIDIRange();
};

extern Settings settings;

#endif
