#ifndef STATE_H
#define STATE_H

#include "NoteLayouts.h"

#include <wiring.h>

// Big state - don't copy
struct BigState {
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
  int pressure;
  int absPressure;
  int modifiedPressure;

  int rotaryEncoderPosition = 0;
  uint32_t loopStartTimeMillis = 0;
};

extern BigState bigState;
extern State state;
extern State prevState;

#endif