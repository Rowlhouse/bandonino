#ifndef STATE_H
#define STATE_H

#include "NoteLayouts.h"
#include "PinInputs.h"

#include <wiring.h>

struct Settings;

// Big state - don't copy
struct BigState {
  // The actual note layout
  const byte* noteLayoutLeftOpen = nullptr;
  const byte* noteLayoutRightOpen = nullptr;
  const byte* noteLayoutLeftClose = nullptr;
  const byte* noteLayoutRightClose = nullptr;
  const char* noteLayoutName = nullptr;

  byte activeKeysLeft[PinInputs::keyCountLeft];
  byte activeKeysRight[PinInputs::keyCountRight];

  byte previousActiveKeysLeft[PinInputs::keyCountLeft];
  byte previousActiveKeysRight[PinInputs::keyCountRight];

  uint32_t activeKeysTimeLeft[PinInputs::keyCountLeft];
  uint32_t activeKeysTimeRight[PinInputs::keyCountRight];

  // Indexed by midi. These a reference counted (so if multiple buttons activate the note, then that is tracked)
  byte playingNotesLeft[127];
  byte playingNotesRight[127];
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
  float absPressure = 0.0f; // clamped to 0 and 1
  float modifiedPressure = 0.0f; // clamped to 0 and 1

  int midiPanLeft = -1;
  int midiPanRight = -1;
  int midiVolumeLeft = -1; // scaled to 0-127
  int midiVolumeRight = -1; // scaled to 0-127

  int rotaryEncoderPosition = 0;
  bool rotaryEncoderPressed = false;
  uint32_t loopStartTimeMillis = 0;
};

extern BigState bigState;
extern State state;
extern State prevState;

#endif