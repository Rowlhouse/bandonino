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

BigState bigState;
State state;
State prevState;

// Config
const int keyReadDelayTime = 3;  // microseconds

bool runHardwareTest = true;
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
void tickRotaryEncoderISR()
{
  rotaryEncoder.tick(); // just call tick() to check the state.
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

// LCD screen related variables
byte upArrow[] = { B00100, B01110, B11111, B00100, B00100, B00100, B00100, B00000 };      // Define an up arrow glyph in binary to send to the LCD
byte downArrow[] = { B00100, B00100, B00100, B00100, B11111, B01110, B00100, B00000 };    // Define a down arrow glyph in binary to send to the LCD
byte recordLeft[] = { B00011, B01111, B01100, B11001, B11001, B01100, B01111, B00011 };   // Define left side of recording glyph in binary to send to the LCD
byte recordRight[] = { B11000, B11110, B00110, B10011, B10011, B00110, B11110, B11000 };  // Define right side of recording glyph in binary to send to the LCD
byte pauseLeft[] = { B00110, B00110, B00110, B00110, B00110, B00110, B00110, B00000 };    // Define left side of pause glyph in binary to send to the LCD
byte pauseRight[] = { B01100, B01100, B01100, B01100, B01100, B01100, B01100, B00000 };   // Define right side of pause glyph in binary to send to the LCD
byte loopLeft[] = { B00000, B00000, B00010, B00100, B00010, B00000, B00000, B00000 };     // Define left side of loop track active glyph in binary to send to the LCD
byte loopRight[] = { B00000, B00000, B01000, B00100, B01000, B00000, B00000, B00000 };    // Define right side of loop track active glyph in binary to send to the LCD

//====================================================================================================
void setup() {
  Serial.begin(38400);

  SyncNoteLayout();

  // Set pin modes - initially all LOW
  initInputPins(PinInputs::columnPinsLeft, PinInputs::columnCountLeft, INPUT);
  initInputPins(PinInputs::columnPinsRight, PinInputs::columnCountRight, INPUT);
  initInputPins(PinInputs::rowPinsLeft, PinInputs::rowCountLeft, INPUT);
  initInputPins(PinInputs::rowPinsRight, PinInputs::rowCountRight, INPUT);

  initKeys(bigState.activeKeysLeft, PinInputs::keyCountLeft);
  initKeys(bigState.activeKeysRight, PinInputs::keyCountRight);

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

  // Inputs needs to be processed before the menus
  readRotaryEncoder();
  readAllKeys();

  // Need to update menu here so if things change then settings != prevSettings
  updateMenu(settings, state);

  SyncNoteLayout();

  if (settings.forceBellows == 0) {
    readPressure();
    updateBellows();
  } else {
    state.bellowsOpening = settings.forceBellows == 1 ? 1 : -1;
    state.midiVolume = 127;
    if (state.midiVolume != prevState.midiVolume) {
      usbMIDI.sendControlChange(0x07, state.midiVolume, settings.midiChannelLeft);
      usbMIDI.sendControlChange(0x07, state.midiVolume, settings.midiChannelRight);
    }
  }

  playAllButtons();

  updatePan();

  if (runHardwareTest)
    hardwareTest();
}

//====================================================================================================
void readPressure() {
  while (!loadcell.is_ready()) {
  }
  state.loadReading = loadcell.read();

  int col = 0;
  int row = 2;
  int iKey = INDEX_RIGHT(row, col);
  if (bigState.activeKeysRight[iKey]) {
    Serial.println("Zeroing load cell");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Resetting bellows");
    display.display();
    delay(1000);
    state.zeroLoadReading = state.loadReading;
    forceMenuRefresh();
  }
  state.pressure = -((state.loadReading - state.zeroLoadReading) * (settings.pressureGain / 100.0f)) / 250000.0f;
  // Serial.println(state.pressure);
}

//====================================================================================================
void updateBellows() {
  if (state.midiVolume == 0 || state.pressure == 0) {  //Bellows stopped
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

  state.midiVolume = std::min((int)(128 * state.modifiedPressure), 127);

  if (state.midiVolume != prevState.midiVolume) {
    usbMIDI.sendControlChange(0x07, state.midiVolume, settings.midiChannelLeft);
    usbMIDI.sendControlChange(0x07, state.midiVolume, settings.midiChannelRight);
  }
}

//====================================================================================================
void updatePan() {
  // Pan control (coarse). 0 is supposedly hard left, 64 center, 127 is hard right
  // That's weird, as it means there's a different range on left and right!
  state.midiPanLeft = 63 + (settings.panLeft * 63) / 100;
  state.midiPanRight = 64 + (settings.panRight * 63) / 100;
  if (state.midiPanLeft != prevState.midiPanLeft) {
    usbMIDI.sendControlChange(10, state.midiPanLeft, settings.midiChannelLeft);
    Serial.printf("PanLeft = %d\n", state.midiPanLeft);
  }
  if (state.midiPanRight != prevState.midiPanRight) {
    usbMIDI.sendControlChange(10, state.midiPanRight, settings.midiChannelRight);
    Serial.printf("PanRight = %d\n", state.midiPanRight);
  }
}

//====================================================================================================
void playNote(byte midiNote, byte velocity, const int midiChannel, byte playingNotes[]) {
  if (midiNote > 0) {
    usbMIDI.sendNoteOn(midiNote, velocity, midiChannel);
    if (velocity > 0) {
      ++playingNotes[midiNote];
    }
  }
}

//====================================================================================================
void stopNote(byte midiNote, byte velocity, const int midiChannel, byte playingNotes[]) {
  if (midiNote > 0) {
    if (playingNotes[midiNote] > 0)
      --playingNotes[midiNote];
    if (playingNotes[midiNote] <= 0)
      usbMIDI.sendNoteOff(midiNote, velocity, midiChannel);
  }
}

//====================================================================================================
void stopAllNotes() {
  // Serial.println("All notes off");
  usbMIDI.sendControlChange(0x7B, 0, settings.midiChannelLeft);
  usbMIDI.sendControlChange(0x7B, 0, settings.midiChannelRight);
  for (int iKey = 0; iKey != PinInputs::keyCountLeft; ++iKey)
    bigState.previousActiveKeysLeft[iKey] = 0;
  for (int iKey = 0; iKey != PinInputs::keyCountRight; ++iKey)
    bigState.previousActiveKeysRight[iKey] = 0;
  for (int midi = settings.midiMin; midi <= settings.midiMax; ++midi) {
    bigState.playingNotesLeft[midi] = 0;
    bigState.playingNotesRight[midi] = 0;
  }
}

//====================================================================================================
// TODO Detect changing bellows direction and stop all notes - otherwise if we change key at the
// same time, playing notes will get out of sync
void playButtons(const byte activeKeys[], byte previousActiveKeys[], const int keyCount, const int midiChannel, const byte* noteLayoutOpen, const byte* noteLayoutClose, byte playingNotes[]) {
  for (int iKey = 0; iKey != keyCount; ++iKey) {
    if (activeKeys[iKey] && !previousActiveKeys[iKey]) {
      // Start playing
      if (state.bellowsOpening > 0)
        playNote(noteLayoutOpen[iKey], 0x7f, midiChannel, playingNotes);
      else if (state.bellowsOpening < 0)
        playNote(noteLayoutClose[iKey], 0x7f, midiChannel, playingNotes);
      else
        continue;
    } else if (!activeKeys[iKey] && previousActiveKeys[iKey]) {
      // Stop playing
      if (state.bellowsOpening > 0)
        stopNote(noteLayoutOpen[iKey], 0x00, midiChannel, playingNotes);
      else if (state.bellowsOpening < 0)
        stopNote(noteLayoutClose[iKey], 0x00, midiChannel, playingNotes);
    }
    previousActiveKeys[iKey] = activeKeys[iKey];
  }
}

//====================================================================================================
void playAllButtons() {
  playButtons(bigState.activeKeysLeft, bigState.previousActiveKeysLeft, PinInputs::keyCountLeft,
              settings.midiChannelLeft, bigState.noteLayoutLeftOpen, bigState.noteLayoutLeftClose, bigState.playingNotesLeft);
  playButtons(bigState.activeKeysRight, bigState.previousActiveKeysRight, PinInputs::keyCountRight,
              settings.midiChannelRight, bigState.noteLayoutRightOpen, bigState.noteLayoutRightClose, bigState.playingNotesRight);
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

      if (keyState == LOW && (currentMillis - activeKeysTime[iKey]) > settings.debounceTime) {
        activeKeys[iKey] = 0;
      }
      pinMode(rowPin, INPUT);
    }
    pinMode(columnPin, INPUT);
  }
}

//====================================================================================================
void readAllKeys() {
  readKeys(PinInputs::rowPinsLeft, PinInputs::columnPinsLeft, bigState.activeKeysLeft, bigState.activeKeysTimeLeft, PinInputs::rowCountLeft, PinInputs::columnCountLeft);
  readKeys(PinInputs::rowPinsRight, PinInputs::columnPinsRight, bigState.activeKeysRight, bigState.activeKeysTimeRight, PinInputs::rowCountRight, PinInputs::columnCountRight);
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
    for (int j = 0; j != PinInputs::columnCountLeft; ++j) {
      for (int i = 0; i < PinInputs::rowCountLeft; i++) {
        int iKey = INDEX_LEFT(i, j);
        Serial.print(bigState.activeKeysLeft[iKey]);
        Serial.print(" (");
        Serial.print(state.bellowsOpening > 0 ? bigState.noteLayoutLeftOpen[iKey] : bigState.noteLayoutLeftClose[iKey]);
        Serial.print(")");
        Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println("Keys right");
    for (int j = 0; j != PinInputs::columnCountRight; ++j) {
      for (int i = 0; i < PinInputs::rowCountRight; i++) {
        int iKey = INDEX_RIGHT(i, j);
        Serial.print(bigState.activeKeysRight[iKey]);
        Serial.print(" (");
        Serial.print(state.bellowsOpening > 0 ? bigState.noteLayoutRightOpen[iKey] : bigState.noteLayoutRightClose[iKey]);
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
      if (bigState.playingNotesLeft[i])
        Serial.printf("%s ", midiNoteNames[i]);
    }
    Serial.println();
    Serial.print("Playing notes right: ");
    for (int i = settings.midiMin; i <= settings.midiMax; ++i) {
      if (bigState.playingNotesRight[i])
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
