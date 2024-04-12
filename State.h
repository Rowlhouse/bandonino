#ifndef STATE_H
#define STATE_H

#include "NoteLayouts.h"
#include "PinInputs.h"

#include <wiring.h>

struct Settings;

// Big state - don't copy
struct BigState {
  // The actual note layout
  NoteLayout noteLayout;

  byte activeKeysLeft[PinInputs::keyCounts[LEFT]];
  byte activeKeysRight[PinInputs::keyCounts[RIGHT]];
  byte* activeKeys(int side) {
    return side ? activeKeysRight : activeKeysLeft;
  };

  byte previousActiveKeysLeft[PinInputs::keyCounts[LEFT]];
  byte previousActiveKeysRight[PinInputs::keyCounts[RIGHT]];
  byte* previousActiveKeys(int side) {
    return side ? previousActiveKeysRight : previousActiveKeysLeft;
  };

  uint32_t activeKeysTimeLeft[PinInputs::keyCounts[LEFT]];
  uint32_t activeKeysTimeRight[PinInputs::keyCounts[RIGHT]];
  uint32_t* activeKeysTimes(int side) {
    return side ? activeKeysTimeRight : activeKeysTimeLeft;
  };

  // Indexed by midi. These a reference counted (so if multiple buttons activate the note, then that is tracked)
  byte playingNotes[2][127];
};

// State can be copied and checked for changes
struct State {
  // opening = 1, stopped = 0, closing = -1
  int bellowsOpening = 1;

  // Raw load cell data
  long zeroLoadReading;
  long loadReading;

  // Pressures - converted using the gain
  float pressure = 0.0f;
  float absPressure = 0.0f;       // clamped to 0 and 1
  float modifiedPressure = 0.0f;  // clamped to 0 and 1

  int midiPans[2] = { -1, -1 };
  int midiVolumes[2] = { -1, -1 };  // scaled to 0-127
  int midiInstruments[2] = { 1, 1 };

  int rotaryEncoderPosition = 0;
  bool rotaryEncoderPressed = false;
  uint32_t loopStartTimeMillis = 0;
};

extern BigState bigState;
extern State state;
extern State prevState;

#endif