// https://www.mathertel.de/Arduino/RotaryEncoderLibrary.aspx
#include <RotaryEncoder.h>

// https://github.com/bogde/HX711
#include <HX711.h>

#include "NoteNames.h"
#include "NoteLayouts.h"
#include "PinInputs.h"
#include "Settings.h"
#include "State.h"
#include "Menu.h"
#include "Metronome.h"

// We don't have a State.cpp file, so put these here
BigState gBigState;
State gState;
State gPrevState;

// Config
// This is the wait (microseconds) between writing to the column and then reading from the rows.
const int sKeyReadDelayTime = 3;

bool runHardwareTest = false;
bool showKeys = false;
bool showBellows = false;
bool flashLED = true;
bool showRot = false;
bool showPlayingNotes = false;

//====================================================================================================
// Rotary encoder pins and library configuration
RotaryEncoder rotaryEncoder(ROTARY_PIN1, ROTARY_PIN2, RotaryEncoder::LatchMode::FOUR3);
// This interrupt routine will be called on any change of one of the input signals
void tickRotaryEncoderISR() {
  rotaryEncoder.tick();  // just call tick() to check the gState.
}

//====================================================================================================
HX711 loadcell;
const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;

unsigned long lastHardwareTestPrintTime;  // Rate limit printing of hardwareTest() info to serial monitor

//====================================================================================================
void initInputPins(const byte pins[], byte pinCount, uint8_t mode) {
  for (int iPin = 0; iPin != pinCount; ++iPin)
    pinMode(pins[iPin], mode);
}

//====================================================================================================
void initKeys(byte activeKeys[], int keyCount) {
  for (int iKey = 0; iKey != keyCount; ++iKey)
    activeKeys[iKey] = 0;
}

// Used to force a periodic sync
const int SYNC_VALUE = -1234;

//====================================================================================================
inline int convertFractionToMidi(float frac) {
  return std::clamp((int)(128 * frac), 0, 127);
}

//====================================================================================================
inline int convertPercentToMidi(int percent) {
  return convertFractionToMidi(percent / 100.0f);
}

//====================================================================================================
void setup() {
  Serial.begin(38400);

  syncNoteLayout();

  // Set pin modes - initially all LOW
  initInputPins(PinInputs::columnPinsLeft, PinInputs::columnCounts[LEFT], INPUT);
  initInputPins(PinInputs::columnPinsRight, PinInputs::columnCounts[RIGHT], INPUT);
  initInputPins(PinInputs::rowPinsLeft, PinInputs::rowCounts[LEFT], INPUT);
  initInputPins(PinInputs::rowPinsRight, PinInputs::rowCounts[RIGHT], INPUT);

  initKeys(gBigState.mActiveKeysLeft, PinInputs::keyCounts[LEFT]);
  initKeys(gBigState.mActiveKeysRight, PinInputs::keyCounts[RIGHT]);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLUP);

  // Initialise the loadcell
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  while (!loadcell.is_ready()) {
  }
  //Zero scale
  gState.mZeroLoadReading = loadcell.read();
  gState.mPressure = 0;

  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN1), tickRotaryEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN2), tickRotaryEncoderISR, CHANGE);

  initMenu();
}

//====================================================================================================
void readRotaryEncoder() {
  // Don't call rotaryEncoder.tick() because it's not safe - it's getting called in the interrupt
  gState.mRotaryEncoderPosition = rotaryEncoder.getPosition();
  gState.mRotaryEncoderPressed = !digitalRead(ROTARY_ENCODER_BUTTON_PIN);
}

//====================================================================================================
void loop() {
  gPrevState = gState;
  gState.mLoopStartTimeMillis = millis();

  // Periodically force the pan/volume to be sent, in case the receiving device wasn't plugged in when we last sent it!
  static uint32_t lastMidiSyncTime = 0;
  if (gState.mLoopStartTimeMillis > lastMidiSyncTime + 1000) {
    for (int side = 0; side != 2; ++side) {
      gPrevState.mMidiPans[side] = SYNC_VALUE;
      gPrevState.mMidiVolumes[side] = SYNC_VALUE;
      gPrevState.mMidiInstruments[side] = SYNC_VALUE;
    }
    lastMidiSyncTime = gState.mLoopStartTimeMillis;
  }

  // Inputs needs to be processed before the menus
  readRotaryEncoder();

  readAllKeys();

  updateMenu();

  syncNoteLayout();

  updateBellows();

  updateMidi();

  playAllButtons();

  updateMetronome();

  if (runHardwareTest)
    hardwareTest();
}

//====================================================================================================
void readPressure() {
  while (!loadcell.is_ready()) {
  }
  gState.mLoadReading = loadcell.read();
  gState.mPressure = -((gState.mLoadReading - gState.mZeroLoadReading) * (gSettings.pressureGain / 100.0f)) / 500000.0f;
}

//====================================================================================================
void updateBellows() {
  if (gSettings.forceBellows == 0) {
    readPressure();

    // Send the pressure to modulate volume
    gState.mAbsPressure = std::min(fabsf(gState.mPressure), 1.0f);  //Absolute Channel Pressure

    float a25 = gSettings.attack25 / 100.0f;
    float a50 = gSettings.attack50 / 100.0f;
    float a75 = gSettings.attack75 / 100.0f;

    if (gState.mAbsPressure < 0.25f) {
      gState.mModifiedPressure = (gState.mAbsPressure * a25) / 0.25f;
    } else if (gState.mAbsPressure < 0.5f) {
      gState.mModifiedPressure = a25 + ((gState.mAbsPressure - 0.25f) * (a50 - a25)) / 0.25f;
    } else if (gState.mAbsPressure < 0.75f) {
      gState.mModifiedPressure = a50 + ((gState.mAbsPressure - 0.5f) * (a75 - a50)) / 0.25f;
    } else {
      gState.mModifiedPressure = a75 + ((gState.mAbsPressure - 0.75f) * (1.0f - a75)) / 0.25f;
    }

    // Handle bellows reversals
    if (gState.mPressure == 0) {  //Bellows stopped
      gState.mBellowsState = BELLOWS_STATE_STATIONARY;
      if (gPrevState.mPressure != 0) {  //Bellows were not previously stopped
        stopAllNotes();               //All Notes Off
      }
    } else {                     //Bellows not stopped
      if (gState.mPressure < 0) {  //Pull
        gState.mBellowsState = BELLOWS_STATE_OPENING;
        if (gPrevState.mPressure >= 0) {  //Pull and Previously Push or stopped
          stopAllNotes();               //All Notes Off
        }
      }
      if (gState.mPressure > 0) {  //Push
        gState.mBellowsState = BELLOWS_STATE_CLOSING;
        if (gPrevState.mPressure <= 0) {  //Push and Previously Pull or stopped
          stopAllNotes();               //All Notes Off
        }
      }
    }

    // If the quantized volume is zero, force that to show as no bellows movement
    if (gState.mMidiVolumes[LEFT] == 0 && gState.mMidiVolumes[RIGHT] == 0)
      gState.mBellowsState = BELLOWS_STATE_STATIONARY;

  } else {
    gState.mModifiedPressure = 1.0f;
    gState.mAbsPressure = 1.0f;

    gState.mBellowsState = gSettings.forceBellows == 1 ? BELLOWS_STATE_OPENING : BELLOWS_STATE_CLOSING;
  }

  for (int side = 0; side != 2; ++side) {
    if (gSettings.expressions[side] == EXPRESSION_VOLUME) {
      float volume = gState.mModifiedPressure * gSettings.levels[side] / 100.0f;
      gState.mMidiVolumes[side] = std::min((int)(128 * volume), 127);
    } else {
      gState.mMidiVolumes[side] = 127;
    }

    if (gState.mMidiVolumes[side] != gPrevState.mMidiVolumes[side])
      usbMIDI.sendControlChange(0x07, gState.mMidiVolumes[side], gSettings.midiChannels[side]);
  }
}

//====================================================================================================
void updateMidi() {
  // Pan control (coarse). 0 is supposedly hard left, 64 center, 127 is hard right
  // That's weird, as it means there's a different range on left and right!
  for (int side = 0; side != 2; ++side) {
    gState.mMidiPans[side] = 64 + (gSettings.pans[side] * 63) / 100;
    if (gState.mMidiPans[side] != gPrevState.mMidiPans[side]) {
      usbMIDI.sendControlChange(10, gState.mMidiPans[side], gSettings.midiChannels[side]);
      if (gPrevState.mMidiPans[side] != SYNC_VALUE)
        Serial.printf("Pan %d = %d\n", side, gState.mMidiPans[side]);
    }

    gState.mMidiInstruments[side] = gSettings.midiInstruments[side];
    if (gState.mMidiInstruments[side] != gPrevState.mMidiInstruments[side]) {
      if (gState.mMidiInstruments[side] != 0)
        usbMIDI.sendProgramChange(gState.mMidiInstruments[side], gSettings.midiChannels[side]);
    }
  }
}

//====================================================================================================
void playNote(int midiNote, byte velocity, const int midiChannel, byte playingNotes[]) {
  if (midiNote > 0 && midiNote <= 127) {
    usbMIDI.sendNoteOn(midiNote, velocity, midiChannel);
    if (velocity > 0) {
      ++playingNotes[midiNote];
    }
  }
}

//====================================================================================================
void stopNote(int midiNote, byte velocity, const int midiChannel, byte playingNotes[]) {
  if (midiNote > 0 && midiNote <= 127) {
    if (playingNotes[midiNote] > 0)
      --playingNotes[midiNote];
    if (playingNotes[midiNote] <= 0)
      usbMIDI.sendNoteOff(midiNote, velocity, midiChannel);
  }
}

//====================================================================================================
void stopAllNotes() {
  // Serial.println("All notes off");
  for (int side = 0; side != 2; ++side) {
    usbMIDI.sendControlChange(0x7B, 0, gSettings.midiChannels[side]);
    for (int iKey = 0; iKey != PinInputs::keyCounts[side]; ++iKey)
      gBigState.previousActiveKeys(side)[iKey] = 0;
    for (int midi = gSettings.midiMin; midi <= gSettings.midiMax; ++midi) {
      gBigState.mPlayingNotes[side][midi] = 0;
    }
  }
}

//====================================================================================================
int getMidiNoteForKey(int iKey, const byte* noteLayoutOpen, const byte* noteLayoutClose, int transpose) {
  if (gState.mBellowsState == BELLOWS_STATE_STATIONARY)
    return -1;
  int midiNote = gState.mBellowsState == BELLOWS_STATE_OPENING ? noteLayoutOpen[iKey] : noteLayoutClose[iKey];
  if (midiNote > 0 && midiNote <= 127) {
    midiNote += transpose;
    if (midiNote > 0 && midiNote <= 127) {
      return midiNote;
    }
  }
  return -1;
}

//====================================================================================================
void playButtons(
  const byte activeKeys[], byte previousActiveKeys[], const int keyCount, const int midiChannel,
  const byte* noteLayoutOpen, const byte* noteLayoutClose, byte playingNotes[], int velocity, int transpose) {
  for (int iKey = 0; iKey != keyCount; ++iKey) {
    if (activeKeys[iKey] && !previousActiveKeys[iKey]) {
      if (gState.mBellowsState != BELLOWS_STATE_STATIONARY) {
        // Start playing, but only if there is some bellows action.
        playNote(getMidiNoteForKey(iKey, noteLayoutOpen, noteLayoutClose, transpose), velocity, midiChannel, playingNotes);
        // Only update previous activity if there is bellows motion - otherwise pressing a key with 
        // the bellows stationary can result in losing the note.
        previousActiveKeys[iKey] = activeKeys[iKey];
      }
    } else if (!activeKeys[iKey] && previousActiveKeys[iKey]) {
      // Stop playing
      stopNote(getMidiNoteForKey(iKey, noteLayoutOpen, noteLayoutClose, transpose), 0, midiChannel, playingNotes);
      previousActiveKeys[iKey] = activeKeys[iKey];
    }
  }
}

//====================================================================================================
int getVelocity(int side) {
  if (gSettings.expressions[side] == EXPRESSION_VOLUME)
    return gSettings.maxVelocity[side];
  return convertFractionToMidi(gState.mModifiedPressure * (gSettings.levels[side] / 100.0f) * (gSettings.maxVelocity[side] / 127.0f));
}

//====================================================================================================
void playAllButtons() {
  for (int side = 0; side != 2; ++side) {
    int velocity = getVelocity(side);
    playButtons(gBigState.activeKeys(side), gBigState.previousActiveKeys(side), PinInputs::keyCounts[side], gSettings.midiChannels[side],
                gBigState.mNoteLayout.open(side), gBigState.mNoteLayout.close(side), gBigState.mPlayingNotes[side], velocity, gSettings.transpose[side]);
  }
}

//====================================================================================================
void readKeys(const byte rowPins[], const byte columnPins[], byte activeKeys[], uint32_t activeKeysTime[], int rowCount, int columnCount) {
  uint32_t currentMillis = millis();

  for (int iColumn = 0; iColumn != columnCount; ++iColumn) {
    byte columnPin = columnPins[iColumn];

    pinMode(columnPin, OUTPUT);
    digitalWrite(columnPin, LOW);

    for (int iRow = 0; iRow != rowCount; ++iRow) {
      byte rowPin = rowPins[iRow];
      pinMode(rowPin, INPUT_PULLUP);

      byte iKey = toKeyIndex(iRow, iColumn, rowCount, columnCount);

      if (sKeyReadDelayTime > 0)
        delayMicroseconds(sKeyReadDelayTime);

      byte keyState = !digitalRead(rowPin);

      if (keyState == HIGH) {
        activeKeys[iKey] = 1;
        activeKeysTime[iKey] = currentMillis;
      }

      if (keyState == LOW && int(currentMillis - activeKeysTime[iKey]) >= gSettings.debounceTime) {
        activeKeys[iKey] = 0;
      }
      pinMode(rowPin, INPUT);
    }
    pinMode(columnPin, INPUT);
  }
}

//====================================================================================================
void readAllKeys() {
  for (int side = 0; side != 2; ++side) {
    readKeys(PinInputs::rowPins(side), PinInputs::columnPins(side), gBigState.activeKeys(side), gBigState.activeKeysTimes(side), PinInputs::rowCounts[side], PinInputs::columnCounts[side]);
  }
}

//====================================================================================================
void hardwareTest() {
  static byte counter = 0;

  // Interval between outputing info. -ve early outs.
  int interval = 250;

  if (showRot) {
    if (gPrevState.mRotaryEncoderPosition != gState.mRotaryEncoderPosition) {
      Serial.print("rotary encoder pos:");
      Serial.print(gState.mRotaryEncoderPosition);
      Serial.print(" dir:");
      Serial.println((int)(rotaryEncoder.getDirection()));
    }
  }

  if (flashLED) {
    // This can be used to measure the tick rate - since it cycles every 256 ticks
    analogWrite(LED_BUILTIN, counter += 1);
  }

  if (millis() < lastHardwareTestPrintTime + interval)
    return;

  lastHardwareTestPrintTime = millis();

  if (showBellows) {
    Serial.println("Bellows");
    Serial.println(gState.mPressure);
    Serial.println(gState.mModifiedPressure);
    Serial.println(gState.mBellowsState);
  }

  if (showKeys) {
    Serial.println("Keys left");
    for (int j = 0; j != PinInputs::columnCounts[LEFT]; ++j) {
      for (int i = 0; i < PinInputs::rowCounts[LEFT]; i++) {
        int iKey = INDEX_LEFT(i, j);
        Serial.print(gBigState.mActiveKeysLeft[iKey]);
        Serial.print(" (");
        Serial.print(gState.mBellowsState == BELLOWS_STATE_OPENING ? gBigState.mNoteLayout.mLeftOpen[iKey] : gBigState.mNoteLayout.mLeftClose[iKey]);
        Serial.print(")");
        Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println("Keys right");
    for (int j = 0; j != PinInputs::columnCounts[RIGHT]; ++j) {
      for (int i = 0; i < PinInputs::rowCounts[RIGHT]; i++) {
        int iKey = INDEX_RIGHT(i, j);
        Serial.print(gBigState.mActiveKeysRight[iKey]);
        Serial.print(" (");
        Serial.print(gState.mBellowsState == BELLOWS_STATE_OPENING ? gBigState.mNoteLayout.mRightOpen[iKey] : gBigState.mNoteLayout.mRightClose[iKey]);
        Serial.print(")");
        Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println();
  }

  if (showPlayingNotes) {
    Serial.print("Playing notes left: ");
    for (int i = gSettings.midiMin; i <= gSettings.midiMax; ++i) {
      if (gBigState.mPlayingNotes[LEFT][i])
        Serial.printf("%s ", midiNoteNames[i]);
    }
    Serial.println();
    Serial.print("Playing notes right: ");
    for (int i = gSettings.midiMin; i <= gSettings.midiMax; ++i) {
      if (gBigState.mPlayingNotes[RIGHT][i])
        Serial.printf("%s ", midiNoteNames[i]);
    }
    Serial.println();
  }

  // Serial.println();
  // Serial.print("Rotary Enc Pos: ");
  // Serial.print(rotaryEncoderPosition);
  // Serial.print("\t");
  // Serial.print("Rotary Enc Button: ");
  // Serial.print(rotaryEncoderButton);
  // Serial.print("\t");
  // Serial.print("Top Pot Val: ");
  // Serial.print(topPotValue);
  // Serial.print("\t");
  // Serial.print("Bottom Pot Val: ");
  // Serial.print(bottomPotValue);
  // Serial.print("\t");
  // Serial.print("Foot Switch: ");
  // Serial.print(footPedalButton);
  // Serial.println();
}
