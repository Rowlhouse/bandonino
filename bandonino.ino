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

BigState bigState;
State state;
State prevState;

// Config
const int keyReadDelayTime = 3;  // microseconds

bool runHardwareTest = false;
bool showKeys = false;
bool showBellows = false;
bool flashLED = true;
bool showRot = false;
bool showPlayingNotes = false;

bool showRawLoadCellReading = false;

//====================================================================================================
// Rotary encoder pins and library configuration
#define ROTARY_PIN1 A9
#define ROTARY_PIN2 A8
RotaryEncoder rotaryEncoder(ROTARY_PIN1, ROTARY_PIN2, RotaryEncoder::LatchMode::FOUR3);
// This interrupt routine will be called on any change of one of the input signals
void tickRotaryEncoderISR() {
  rotaryEncoder.tick();  // just call tick() to check the state.
}

// Digital input pins
const byte rotaryEncoderButtonPin = 17;

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

  initKeys(bigState.activeKeysLeft, PinInputs::keyCounts[LEFT]);
  initKeys(bigState.activeKeysRight, PinInputs::keyCounts[RIGHT]);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rotaryEncoderButtonPin, INPUT_PULLUP);

  // Initialise the loadcell
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  while (!loadcell.is_ready()) {
  }
  //Zero scale
  state.zeroLoadReading = loadcell.read();
  state.pressure = 0;

  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN1), tickRotaryEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN2), tickRotaryEncoderISR, CHANGE);

  initMenu();
}

//====================================================================================================
void readRotaryEncoder() {
  // rotaryEncoder.tick();
  state.rotaryEncoderPosition = rotaryEncoder.getPosition();

  state.rotaryEncoderPressed = !digitalRead(rotaryEncoderButtonPin);
}

//====================================================================================================
void loop() {
  prevState = state;
  state.loopStartTimeMillis = millis();

  // Periodically force the pan/volume to be sent, in case the receiving device wasn't plugged in when we last sent it!
  static uint32_t lastMidiSyncTime = 0;
  if (state.loopStartTimeMillis > lastMidiSyncTime + 1000) {
    for (int side = 0; side != 2; ++side) {
      prevState.midiPans[side] = SYNC_VALUE;
      prevState.midiVolumes[side] = SYNC_VALUE;
      prevState.midiInstruments[side] = SYNC_VALUE;
    }
    lastMidiSyncTime = state.loopStartTimeMillis;
  }

  // Inputs needs to be processed before the menus
  readRotaryEncoder();
  readAllKeys();

  // Need to update menu here so if things change then settings != prevSettings
  updateMenu(settings, state);

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
  state.loadReading = loadcell.read();

#if 0
  int col = 0;
  int row = 2;
  int iKey = INDEX_RIGHT(row, col);
  if (bigState.activeKeysRight[iKey]) {
    zeroBellows();
  }
#endif
  state.pressure = -((state.loadReading - state.zeroLoadReading) * (settings.pressureGain / 100.0f)) / 500000.0f;
  // Serial.println(state.pressure);
}

//====================================================================================================
void updateBellows() {
  if (settings.forceBellows == 0) {
    readPressure();

    // Send the pressure to modulate volume
    state.absPressure = std::min(fabsf(state.pressure), 1.0f);  //Absolute Channel Pressure

    float a25 = settings.attack25 / 100.0f;
    float a50 = settings.attack50 / 100.0f;
    float a75 = settings.attack75 / 100.0f;

    if (state.absPressure < 0.25f) {
      state.modifiedPressure = (state.absPressure * a25) / 0.25f;
    } else if (state.absPressure < 0.5f) {
      state.modifiedPressure = a25 + ((state.absPressure - 0.25f) * (a50 - a25)) / 0.25f;
    } else if (state.absPressure < 0.75f) {
      state.modifiedPressure = a50 + ((state.absPressure - 0.5f) * (a75 - a50)) / 0.25f;
    } else {
      state.modifiedPressure = a75 + ((state.absPressure - 0.75f) * (1.0f - a75)) / 0.25f;
    }

    // Handle bellows reversals
    if (state.pressure == 0) {  //Bellows stopped
      state.bellowsOpening = 0;
      if (prevState.pressure != 0) {  //Bellows were not previously stopped
        stopAllNotes();               //All Notes Off
      }
    } else {                     //Bellows not stopped
      if (state.pressure < 0) {  //Pull
        state.bellowsOpening = 1;
        if (prevState.pressure >= 0) {  //Pull and Previously Push or stopped
          stopAllNotes();               //All Notes Off
        }
      }
      if (state.pressure > 0) {  //Push
        state.bellowsOpening = -1;
        if (prevState.pressure <= 0) {  //Push and Previously Pull or stopped
          stopAllNotes();               //All Notes Off
        }
      }
    }

    // If the quantized volume is zero, force that to show as no bellows movement
    if (state.midiVolumes[LEFT] == 0 && state.midiVolumes[RIGHT] == 0)
      state.bellowsOpening = 0;

  } else {
    state.modifiedPressure = 1.0f;
    state.absPressure = 1.0f;

    state.bellowsOpening = settings.forceBellows == 1 ? 1 : -1;
  }

  for (int side = 0; side != 2; ++side) {
    if (settings.expressions[side] == EXPRESSION_VOLUME) {
      float volume = state.modifiedPressure * settings.levels[side] / 100.0f;
      state.midiVolumes[side] = std::min((int)(128 * volume), 127);
    } else {
      state.midiVolumes[side] = 127;
    }

    if (state.midiVolumes[side] != prevState.midiVolumes[side])
      usbMIDI.sendControlChange(0x07, state.midiVolumes[side], settings.midiChannels[side]);
  }
}

//====================================================================================================
void updateMidi() {
  // Pan control (coarse). 0 is supposedly hard left, 64 center, 127 is hard right
  // That's weird, as it means there's a different range on left and right!
  for (int side = 0; side != 2; ++side) {
    state.midiPans[side] = 64 + (settings.pans[side] * 63) / 100;
    if (state.midiPans[side] != prevState.midiPans[side]) {
      usbMIDI.sendControlChange(10, state.midiPans[side], settings.midiChannels[side]);
      if (prevState.midiPans[side] != SYNC_VALUE)
        Serial.printf("Pan %d = %d\n", side, state.midiPans[side]);
    }

    state.midiInstruments[side] = settings.midiInstruments[side];
    if (state.midiInstruments[side] != prevState.midiInstruments[side]) {
      if (state.midiInstruments[side] != 0)
        usbMIDI.sendProgramChange(state.midiInstruments[side], settings.midiChannels[side]);
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
    usbMIDI.sendControlChange(0x7B, 0, settings.midiChannels[side]);
    for (int iKey = 0; iKey != PinInputs::keyCounts[side]; ++iKey)
      bigState.previousActiveKeys(side)[iKey] = 0;
    for (int midi = settings.midiMin; midi <= settings.midiMax; ++midi) {
      bigState.playingNotes[side][midi] = 0;
    }
  }
}

//====================================================================================================
int getMidiNoteForKey(int iKey, const byte* noteLayoutOpen, const byte* noteLayoutClose) {
  if (state.bellowsOpening == 0)
    return -1;
  int midiNote = state.bellowsOpening > 0 ? noteLayoutOpen[iKey] : noteLayoutClose[iKey];
  if (midiNote > 0 && midiNote <= 127) {
    midiNote += settings.transpose;
    if (midiNote > 0 && midiNote <= 127) {
      return midiNote;
    }
  }
  return -1;
}

//====================================================================================================
void playButtons(
  const byte activeKeys[],
  byte previousActiveKeys[],
  const int keyCount,
  const int midiChannel,
  const byte* noteLayoutOpen,
  const byte* noteLayoutClose,
  byte playingNotes[],
  int velocity) {
  for (int iKey = 0; iKey != keyCount; ++iKey) {
    if (activeKeys[iKey] && !previousActiveKeys[iKey]) {
      // Start playing
      playNote(getMidiNoteForKey(iKey, noteLayoutOpen, noteLayoutClose), velocity, midiChannel, playingNotes);
    } else if (!activeKeys[iKey] && previousActiveKeys[iKey]) {
      // Stop playing
      stopNote(getMidiNoteForKey(iKey, noteLayoutOpen, noteLayoutClose), 0, midiChannel, playingNotes);
    }
    previousActiveKeys[iKey] = activeKeys[iKey];
  }
}

//====================================================================================================
int getVelocity(int side) {
  if (settings.expressions[side] == EXPRESSION_VOLUME)
    return settings.maxVelocity[side];
  return convertFractionToMidi(state.modifiedPressure * (settings.levels[side] / 100.0f) * (settings.maxVelocity[side] / 127.0f));
}

//====================================================================================================
void playAllButtons() {
  for (int side = 0; side != 2; ++side) {
    int velocity = getVelocity(side);
    playButtons(bigState.activeKeys(side), bigState.previousActiveKeys(side), PinInputs::keyCounts[side],
                settings.midiChannels[side], bigState.noteLayout.open(side), bigState.noteLayout.close(side), bigState.playingNotes[side], velocity);
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

      if (keyReadDelayTime > 0)
        delayMicroseconds(keyReadDelayTime);

      byte keyState = !digitalRead(rowPin);

      if (keyState == HIGH) {
        activeKeys[iKey] = 1;
        activeKeysTime[iKey] = currentMillis;
      }

      if (keyState == LOW && int(currentMillis - activeKeysTime[iKey]) > settings.debounceTime) {
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
    readKeys(PinInputs::rowPins(side), PinInputs::columnPins(side), bigState.activeKeys(side), bigState.activeKeysTimes(side), PinInputs::rowCounts[side], PinInputs::columnCounts[side]);
  }
}

//====================================================================================================
void hardwareTest() {
  static byte counter = 0;

  // Interval between outputing info. -ve early outs.
  int interval = 250;

  if (showRot) {
    if (prevState.rotaryEncoderPosition != state.rotaryEncoderPosition) {
      Serial.print("rotary encoder pos:");
      Serial.print(state.rotaryEncoderPosition);
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
    Serial.println(state.pressure);
    Serial.println(state.modifiedPressure);
    Serial.println(state.bellowsOpening);
  }

  if (showKeys) {
    Serial.println("Keys left");
    for (int j = 0; j != PinInputs::columnCounts[LEFT]; ++j) {
      for (int i = 0; i < PinInputs::rowCounts[LEFT]; i++) {
        int iKey = INDEX_LEFT(i, j);
        Serial.print(bigState.activeKeysLeft[iKey]);
        Serial.print(" (");
        Serial.print(state.bellowsOpening > 0 ? bigState.noteLayout.leftOpen[iKey] : bigState.noteLayout.leftClose[iKey]);
        Serial.print(")");
        Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println("Keys right");
    for (int j = 0; j != PinInputs::columnCounts[RIGHT]; ++j) {
      for (int i = 0; i < PinInputs::rowCounts[RIGHT]; i++) {
        int iKey = INDEX_RIGHT(i, j);
        Serial.print(bigState.activeKeysRight[iKey]);
        Serial.print(" (");
        Serial.print(state.bellowsOpening > 0 ? bigState.noteLayout.rightOpen[iKey] : bigState.noteLayout.rightClose[iKey]);
        Serial.print(")");
        Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println();
  }

  if (showPlayingNotes) {
    Serial.print("Playing notes left: ");
    for (int i = settings.midiMin; i <= settings.midiMax; ++i) {
      if (bigState.playingNotes[LEFT][i])
        Serial.printf("%s ", midiNoteNames[i]);
    }
    Serial.println();
    Serial.print("Playing notes right: ");
    for (int i = settings.midiMin; i <= settings.midiMax; ++i) {
      if (bigState.playingNotes[RIGHT][i])
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
