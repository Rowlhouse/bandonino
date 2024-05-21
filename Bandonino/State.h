#ifndef STATE_H
#define STATE_H

#include "NoteLayouts.h"
#include "PinInputs.h"

struct Settings;

enum BellowsState {
  BELLOWS_STATE_CLOSING = -1,
  BELLOWS_STATE_STATIONARY = 0,
  BELLOWS_STATE_OPENING = 1
};

// Big state - don't copy
struct BigState {
  // The actual note layout
  NoteLayout mNoteLayout;

  uint8_t mActiveKeysLeft[PinInputs::keyCounts[LEFT]];
  uint8_t mActiveKeysRight[PinInputs::keyCounts[RIGHT]];
  uint8_t* activeKeys(int side) {
    return side ? mActiveKeysRight : mActiveKeysLeft;
  };

  uint8_t mPreviousActiveKeysLeft[PinInputs::keyCounts[LEFT]];
  uint8_t mPreviousActiveKeysRight[PinInputs::keyCounts[RIGHT]];
  uint8_t* previousActiveKeys(int side) {
    return side ? mPreviousActiveKeysRight : mPreviousActiveKeysLeft;
  };

  // This is set to the current time (ms) when the key is being pressed - so it's
  // possible to tell how long ago it has been since the key was released.
  uint32_t mActiveKeysTimeLeft[PinInputs::keyCounts[LEFT]];
  uint32_t mActiveKeysTimeRight[PinInputs::keyCounts[RIGHT]];
  uint32_t* activeKeysTimes(int side) {
    return side ? mActiveKeysTimeRight : mActiveKeysTimeLeft;
  };

  // Indexed by midi. These a reference counted (so if multiple buttons activate the note, then that is tracked)
  uint8_t mPlayingNotes[2][127];
};

// State can be copied and checked for changes
struct State {
  BellowsState mBellowsState = BELLOWS_STATE_STATIONARY;

  // Raw load cell data
  long mLoadReading;

  // Pressures - converted using the gain
  float mPressure = 0.0f;
  float mAbsPressure = 0.0f;       // clamped to 0 and 1
  float mModifiedPressure = 0.0f;  // clamped to 0 and 1

  int mMidiPans[2] = { -1, -1 };
  int mMidiVolumes[2] = { -1, -1 };  // scaled to 0-127
  int mMidiInstruments[2] = { 1, 1 };

  int mRotaryEncoderPosition = 0;
  bool mRotaryEncoderPressed = false;
  uint32_t mLoopStartTimeMillis = 0;

  void readPressure();
};

extern BigState gBigState;
extern State gState;
extern State gPrevState;

#endif