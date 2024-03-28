// SSD1306Ascii v1.3.5
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

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

Settings settings;
BigState bigState;
State state;
State prevState;

// Config
const int keyReadDelayTime = 3;  // microseconds

int prevPanLeft = -1000;
int prevPanRight = -1000;

bool runHardwareTest = true;
bool showKeys = false;
bool showBellows = false;
bool flashLED = true;
bool showRot = true;

bool showRawLoadCellReading = false;

// OLED I2C address and library configuration
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
// Font is 5 wide, but have a space
byte charWidth = 6;

//====================================================================================================
// Rotary encoder pins and library configuration
RotaryEncoder rotaryEncoder(A9, A8, RotaryEncoder::LatchMode::FOUR3);

// Digital input pins
const byte rotaryEncoderButtonPin = 17;

//====================================================================================================

#define USE_LOADCELL
#ifdef USE_LOADCELL
HX711 loadcell;
#endif

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
void scrollInText(int col, int row, const char* text, int ms) {
  const int textLen = strlen(text);
  for (int i = textLen * charWidth; i-- != 0; ) {
    oled.setCursor(col * charWidth + i, row);
    oled.print(text);
    delay(ms);
  }
}

//====================================================================================================
// Display is 128x64 - so 16x8 characters
void setupOLED() {
  Wire.begin();
  Wire.setClock(400000L);                    // Fast mode
  oled.begin(&Adafruit128x64, I2C_ADDRESS);  // OLED type and address
  oled.setFont(font5x7);                     // Set the font type

  oled.clear();
  scrollInText(0, 0, "Bandon.ino ", 3);
  scrollInText(0, 1, "Danny Chapman ", 3);
  delay(1000);
  oled.clear();
}

//====================================================================================================
void setup() {
  setupOLED();
  Serial.begin(38400);

  SetNoteLayout(LAYOUT_MANOURY, settings);

  // Set pin modes - initially all LOW
  initInputPins(PinInputs::columnPinsLeft, PinInputs::columnCountLeft, INPUT);
  initInputPins(PinInputs::columnPinsRight, PinInputs::columnCountRight, INPUT);
  initInputPins(PinInputs::rowPinsLeft, PinInputs::rowCountLeft, INPUT);
  initInputPins(PinInputs::rowPinsRight, PinInputs::rowCountRight, INPUT);

  initKeys(bigState.activeKeysLeft, PinInputs::keyCountLeft);
  initKeys(bigState.activeKeysRight, PinInputs::keyCountRight);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rotaryEncoderButtonPin, INPUT_PULLUP);

#ifdef USE_LOADCELL
  if (settings.useBellows) {
    // Initialise the loadcell
    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    while (!loadcell.is_ready()) {
    }
    //Zero scale
    state.zeroLoadReading = loadcell.read();
    state.pressure = 0;
  }
#endif
}

//====================================================================================================
void displayPlayingNotes(byte playingNotes[], int row) {
  int col = 0;
  for (int i = settings.midiMin; i <= settings.midiMax; ++i) {
    if (playingNotes[i]) {
      oled.setCursor(col * charWidth, row);
      col += oled.printf("%s ", midiNoteNames[i]);
    }
  }
  if (col < 15) {
    oled.setCursor(col * charWidth, row);
    oled.print("                ");
  }
}

//====================================================================================================
void displayAllPlayingNotes() {
  displayPlayingNotes(bigState.playingNotesLeft, 3);
  displayPlayingNotes(bigState.playingNotesRight, 5);
}

//====================================================================================================
void handleRotaryEncoder() {
  rotaryEncoder.tick();
  state.rotaryEncoderPosition = rotaryEncoder.getPosition();
}

//====================================================================================================
void loop() {
  prevState = state;
  state.loopStartTimeMillis = millis();

  readAllKeys();

  adjustPan();

  int col = 5;
  int row = 5;
  int iKey = INDEX_RIGHT(row, col);
  if (bigState.activeKeysRight[iKey]) {
    Serial.println("Toggling bellows");
    delay(1000);
    settings.useBellows = !settings.useBellows;
  }

  if (settings.useBellows) {
    readPressure();
    handleBellows();
  } else {
    usbMIDI.sendControlChange(0x07, 127, settings.midiChannelLeft);
    usbMIDI.sendControlChange(0x07, 127, settings.midiChannelRight);
  }

  handleRotaryEncoder();

  playAllButtons();

  displayAllPlayingNotes();

  // extra info
  oled.setCursor(0, 6);
  oled.printf("P %3d -> %3d %d ", state.absPressure, state.modifiedPressure, state.rotaryEncoderPosition);
  oled.setCursor(0, 7);
  oled.printf("%5.1f  ", 1000.0f / (state.loopStartTimeMillis - prevState.loopStartTimeMillis));

  if (runHardwareTest)
    hardwareTest();

  // With no delay we spam so many midi messages it can cause problems.
  // delay(5);
}

//====================================================================================================
void readPressure() {
#ifdef USE_LOADCELL
  while (!loadcell.is_ready()) {
  }
  state.loadReading = loadcell.read();

  int col = 4;
  int row = 6;
  int iKey = INDEX_RIGHT(row, col);
  if (bigState.activeKeysRight[iKey]) {
    Serial.println("Zeroing load cell");
    delay(1000);
    state.zeroLoadReading = state.loadReading;
  }

  state.pressure = ((state.loadReading - state.zeroLoadReading) * settings.pressureGain) / 100000;

  // Serial.println("Zero reading, load reading, current pressure");
  // Serial.println(state.zeroLoadReading);
  if (showRawLoadCellReading) {
    Serial.printf("Current load reading %d\n", state.loadReading);
  }
  // Serial.println(state.loadReading);
  // Serial.println(state.pressure);

#endif
}

//====================================================================================================
void handleBellows() {
  if (state.pressure == 0) {        //Bellows stopped
    if (prevState.pressure != 0) {  //Bellows were not previously stopped
      stopAllNotes();               //All Notes Off
    }
  } else {                            //Bellows not stopped
    if (state.pressure < 0) {         //Pull
      if (prevState.pressure >= 0) {  //Pull and Previously Push or stopped
        stopAllNotes();               //All Notes Off
      }
    }
    if (state.pressure > 0) {         //Push
      if (prevState.pressure <= 0) {  //Push and Previously Pull or stopped
        stopAllNotes();               //All Notes Off
      }
    }
  }

  // Send the pressure to modulate volume
  state.absPressure = abs(state.pressure);  //Absolute Channel Pressure
  if (state.absPressure > 0x7F) {
    state.absPressure = 0x7F;
  }

  if (state.absPressure < 32) {
    state.modifiedPressure = (state.absPressure * settings.attack1) / 32;
  }
  if (state.absPressure > 31) {
    if (state.absPressure < 64) {
      state.modifiedPressure = settings.attack1 + ((state.absPressure - 31) * (settings.attack2 - settings.attack1)) / 32;
    }
  }
  if (state.absPressure > 63) {
    if (state.absPressure < 95) {
      state.modifiedPressure = settings.attack2 + ((state.absPressure - 63) * (settings.attack3 - settings.attack2)) / 32;
    }
  }
  if (state.absPressure > 94) {
    state.modifiedPressure = settings.attack3 + ((state.absPressure - 95) * (127 - settings.attack3)) / 32;
  }

  if (state.modifiedPressure != prevState.modifiedPressure) {
    usbMIDI.sendControlChange(0x07, state.modifiedPressure, settings.midiChannelLeft);
    usbMIDI.sendControlChange(0x07, state.modifiedPressure, settings.midiChannelRight);
  }
}

//====================================================================================================
void adjustPan() {
  // Pan control (coarse). 0 is supposedly hard left, 64 center, 127 is hard right
  if (settings.panLeft != prevPanLeft) {
    usbMIDI.sendControlChange(10, 63 + (settings.panLeft * 63) / 100, settings.midiChannelLeft);
    prevPanLeft = settings.panLeft;
  }
  if (settings.panRight != prevPanRight) {
    usbMIDI.sendControlChange(10, 64 + (settings.panRight * 63) / 100, settings.midiChannelRight);
    prevPanRight = settings.panRight;
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
              settings.midiChannelLeft, settings.noteLayoutLeftOpen, settings.noteLayoutLeftClose, bigState.playingNotesLeft);
  playButtons(bigState.activeKeysRight, bigState.previousActiveKeysRight, PinInputs::keyCountRight,
              settings.midiChannelRight, settings.noteLayoutRightOpen, settings.noteLayoutRightClose, bigState.playingNotesRight);
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
  }

  if (showKeys) {
    Serial.println("Keys left");
    for (int j = 0; j != PinInputs::columnCountLeft; ++j) {
      for (int i = 0; i < PinInputs::rowCountLeft; i++) {
        int iKey = INDEX_LEFT(i, j);
        Serial.print(bigState.activeKeysLeft[iKey]);
        Serial.print(" (");
        Serial.print(state.bellowsOpening > 0 ? settings.noteLayoutLeftOpen[iKey] : settings.noteLayoutLeftClose[iKey]);
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
        Serial.print(state.bellowsOpening > 0 ? settings.noteLayoutRightOpen[iKey] : settings.noteLayoutRightClose[iKey]);
        Serial.print(")");
        Serial.print("\t");
      }
      Serial.println();
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
