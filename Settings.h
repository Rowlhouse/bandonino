#ifndef SETTINGS_H
#define SETTINGS_H

#include "NoteLayouts.h"

#include <wiring.h>

enum ExpressionType {
  EXPRESSION_TYPE_BREATH,
  EXPRESSION_TYPE_VELOCITY,
  EXPRESSION_TYPE_NUM
};
extern const char* gExpressionTypes[];

struct Settings {
  int slot; // settings slot - e.g. 0 to 9
  int noteLayout = NOTELAYOUTTYPE_MANOURY2;
  int forceBellows = 0;   // 1 means use opening. -1 means use closing. 0 means use the pressure sensor
  int pressureGain = 50;  // Treat as percentage
  int expressionTypes[2] = {EXPRESSION_TYPE_BREATH, EXPRESSION_TYPE_BREATH};

  uint32_t debounceTime = 10;  // milliseconds

  int midiChannels[2] = {1, 2};

  // percentages between -100 and 100
  int pans[2] = {-50, 50};

  // Percentages between 0 and 100
  int levels[2] = {100, 100};

  int showFPS = 0;
  int menuBrightness = 12; // 0 to 0xf

  // Shape the pressure response using attack params. As percentages
  int attack25 = 25;  // output when pressure = 25%
  int attack50 = 50;  // output when pressure = 50%
  int attack75 = 75;  // output when pressure = 75%

  // Things below here are updated automatically
  byte midiMin = 0;
  byte midiMax = 127;

  // Call this to limit the range of midi notes we traverse after changing the layout
  void updateMIDIRange();

  bool writeToCard(const char* filename);
  bool readFromCard(const char* filename);
};

extern Settings settings;

#endif
