#include "Menu.h"

#include "Settings.h"
#include "State.h"
#include "Bellows.h"
#include "NoteNames.h"
#include "Bitmaps.h"

//====================================================================================================
// 1327 128x128 Display
//====================================================================================================
// https://github.com/adafruit/Adafruit_SSD1327
// v 1.0.4
#include <Adafruit_SSD1327.h>

#include <algorithm>
#include <vector>

//====================================================================================================
#define I2C_ADDRESS 0x3D
#define OLED_RESET -1
Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET, 4000000);

// Note that fonts can be generated from https://oleddisplay.squix.ch/#/home
#include "Fonts/FreeSans9pt7b.h"
static const GFXfont* sPageTitleFont = &FreeSans9pt7b;
static const int sCharWidth = 6;  // 5 plus 1 for the space
static const int sCharHeight = 8;

// Screen size in the defautl character size
static const int sScreenCharWidth = 128 / sCharWidth;
static const int sScreenCharHeight = 128 / sCharHeight;

// The height of the top of page contents - i.e. draw using the default font at this y value plus char height to not
// clip the page title
static const int sPageY = 16;

static bool sForceMenuRefresh = false;

//====================================================================================================
// This is a bit hacky, overloading the constructors. Each entry should be customisable as it's
// added, with a clearer definition. Then we wouldn't need the type either.
struct Option {
  typedef void (*Action)();

  enum Type {
    TYPE_OPTION,
    TYPE_ACTION,
    TYPE_NONE
  };

  Option(const char* name, int* value, int minValue, int maxValue, int deltaValue, bool wrap = false, Action action = nullptr)
    : mType(TYPE_OPTION), mName(name), mIntValue(value), mIntMinValue(minValue), mIntMaxValue(maxValue), mIntDeltaValue(deltaValue), mWrap(wrap), mAction(action) {}

  Option(const char* name, int* value, const char** valueStrings, int numStrings, Action action = nullptr)
    : mType(TYPE_OPTION), mName(name), mValueStrings(valueStrings), mIntValue(value), mIntMinValue(0), mIntMaxValue(numStrings - 1), mIntDeltaValue(1), mWrap(true), mAction(action) {}

  Option(const char* name, float* value, float minValue, float maxValue, float deltaValue, Action action = nullptr)
    : mType(TYPE_OPTION), mName(name), mFloatValue(value), mFloatMinValue(minValue), mFloatMaxValue(maxValue), mFloatDeltaValue(deltaValue), mWrap(false), mAction(action) {}

  Option(const char* name, Action action)
    : mType(TYPE_ACTION), mName(name), mAction(action) {}

  Option(Action action = nullptr)
    : mType(TYPE_NONE), mAction(action) {}

  Type mType;

  const char* mName = "";
  const char** mValueStrings = nullptr;

  int* mIntValue = nullptr;
  float* mFloatValue = nullptr;

  int mIntMinValue = 0;
  int mIntMaxValue = 0;
  int mIntDeltaValue = 0;
  float mFloatMinValue = 0;
  float mFloatMaxValue = 0;
  float mFloatDeltaValue = 0;

  bool mWrap = false;

  Action mAction = nullptr;
};

enum SelectionMode {
  SELECTION_MODE_OPTION,
  SELECTION_MODE_VALUE,
  SELECTION_MODE_MAX
};

static const char* sForceBellowsStrings[] = {
  "Sensor", "Open", "Close"
};

struct Page {
  enum Type {
    TYPE_SPLASH,
    TYPE_STATUS,
    TYPE_PLAYING_NOTES,
    TYPE_PLAYING_STAFF,
    TYPE_BELLOWS,
    TYPE_OPTIONS
  };

  Page(Type type, const char* title, const std::vector<Option>& options)
    : mType(type), mTitle(title), mOptions(options) {}
  Page(Type type, const char* title)
    : mType(type), mTitle(title) {}
  Type mType;
  const char* mTitle;
  std::vector<Option> mOptions;
};

std::vector<Page> sPages;

// Track prev and current values to see if things need redrawing
static int sPreviousPageIndex = -1;

static int sPreviousOptionIndex = -1;
static int sCurrentOptionIndex = 0;

// toggle between adjusting option vs value by clicking
static bool sAdjustOption = true;

uint32_t sSplashTime = 0;  // When splash was triggered
const uint32_t SPLASH_DURATION = 2000;

static float sAverageFPS = 0;
static float sWorstFPS = 0;

//====================================================================================================
void forceMenuRefresh() {
  sForceMenuRefresh = true;
  sPreviousOptionIndex = -1;
  sPreviousPageIndex = -1;
}

//====================================================================================================
void showMessage(const char* msg, int time) {
  display.clearDisplay();
  display.setFont(sPageTitleFont);
  display.setCursor(0, 64);
  display.print(msg);
  display.display();
  display.setFont(nullptr);
  delay(time);
  forceMenuRefresh();
}

//====================================================================================================
void saveSettings() {
  Serial.println("Writing gSettings to gSettings.json");
  if (!gSettings.writeToCard())
    Serial.println("Failed to write to gSettings.json");
}

//====================================================================================================
void actionSaveSettings() {
  Serial.println("Save gSettings");
  char filename[32];
  sprintf(filename, "Settings%02d.json", gSettings.slot);
  if (!gSettings.writeToCard(filename))
    Serial.printf("Failed to write gSettings to %s\n", filename);
  else
    showMessage("Saved", 500);
}

//====================================================================================================
// Displays a little countdown prior to measuring the zero value
void resetBellows() {
  Serial.println("Reset bellows");
  display.clearDisplay();

  display.setFont(sPageTitleFont);
  display.setCursor(0, sPageTitleFont->yAdvance);
  display.print("Zero bellows");
  display.display();
  display.setFont(nullptr);

  display.setTextSize(3);
  for (int i = 3; i != 0; --i) {
    display.setCursor(56, 64);
    display.printf("%d", i);
    display.display();
    delay(500);
  }
  display.setTextSize(1);
  display.setTextColor(gSettings.menuBrightness, 0x0);

  zeroBellows();
  forceMenuRefresh();
}

//====================================================================================================
void actionResetBellows() {
  resetBellows();
}

//====================================================================================================
void actionResetSettings() {
  gSettings = Settings();
  showMessage("Reset", 500);
}

//====================================================================================================
void actionLoadBandoneon() {
  gSettings.reset();
  gSettings.midiInstruments[LEFT] = 0;
  gSettings.midiInstruments[RIGHT] = 0;
  gSettings.balance = 10;
  gSettings.stereo = 50;
  showMessage("Bandoneon", 500);
}

//====================================================================================================
void actionLoadConcertina() {
  gSettings.reset();
  gSettings.midiInstruments[LEFT] = 1;
  gSettings.midiInstruments[RIGHT] = 1;
  gSettings.balance = 0;
  gSettings.stereo = 50;
  showMessage("Bandoneon", 500);
}

//====================================================================================================
void actionLoadPiano() {
  gSettings.reset();
  gSettings.midiInstruments[LEFT] = 2;
  gSettings.midiInstruments[RIGHT] = 2;
  gSettings.expressions[LEFT] = EXPRESSION_VELOCITY;
  gSettings.expressions[RIGHT] = EXPRESSION_VELOCITY;
  gSettings.balance = 0;
  gSettings.stereo = 0;
  gSettings.debounceTime = 10;
  showMessage("Piano", 500);
}

//====================================================================================================
void actionLoadBandoPiano() {
  gSettings.reset();
  gSettings.midiInstruments[LEFT] = 2;
  gSettings.midiInstruments[RIGHT] = 0;
  gSettings.expressions[LEFT] = EXPRESSION_VELOCITY;
  gSettings.balance = -20;
  gSettings.stereo = 25;
  gSettings.debounceTime = 10;
  showMessage("BandoPiano", 500);
}

//====================================================================================================
void actionLoadSettings() {
  Serial.println("Load gSettings");
  char filename[32];
  sprintf(filename, "Settings%02d.json", gSettings.slot);
  if (!gSettings.readFromCard(filename))
    Serial.printf("Failed to write gSettings to %s\n", filename);
  else
    showMessage("Loaded", 500);
}

//====================================================================================================
void actionShowFPS() {
  gSettings.showFPS = !gSettings.showFPS;
  sForceMenuRefresh = true;
  sPreviousOptionIndex = -1;
  sPreviousPageIndex = -1;
}

//====================================================================================================
void disableDisplay() {
  if (gSettings.menuDisplayEnabled) {
    gSettings.menuDisplayEnabled = false;
    display.clearDisplay();
    display.display();
    saveSettings();
  }
}

//====================================================================================================
void enableDisplay() {
  if (!gSettings.menuDisplayEnabled) {
    gSettings.menuDisplayEnabled = true;
    forceMenuRefresh();
    saveSettings();
  }
}

//====================================================================================================
void actionToggleDisplay() {
  if (gSettings.menuDisplayEnabled) {
    Serial.println("Toggling display to off");
    disableDisplay();
  } else {
    Serial.println("Toggling display to on");
    enableDisplay();
  }
}

//====================================================================================================
void actionToggleMetronome() {
  gSettings.metronomeEnabled = !gSettings.metronomeEnabled;
}

//====================================================================================================
void scrollInText(int x, int y, const char* text, int ms) {
  for (int x1 = 128; --x1 >= x;) {
    display.setCursor(x1, y);
    display.printf("%s ", text);
    display.display();
    delay(ms);
  }
}

//====================================================================================================
void overlayFPS(int x = 0, int y = 128 - sCharHeight) {
  display.setCursor(x, y);
  display.printf("%5.1f (%5.1f)", sAverageFPS, sWorstFPS);
}

//====================================================================================================
// Display is 128x64 - so 16x8 characters
void initMenu() {
  if (!display.begin(I2C_ADDRESS))
    Serial.println("Unable to initialize OLED");

  // Not sure there's any merit to a blank page, since the display can be turned off by clicking
  // sPages.push_back(Page(Page::TYPE_SPLASH, "Bandon.ino", { Option() }));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Settings", {}));
  sPages.back().mOptions.push_back(Option("Slot", &gSettings.slot, 0, 9, 1));
  sPages.back().mOptions.push_back(Option("Save", &actionSaveSettings));
  sPages.back().mOptions.push_back(Option("Load", &actionLoadSettings));
  sPages.back().mOptions.push_back(Option("Bandoneon", &actionLoadBandoneon));
  sPages.back().mOptions.push_back(Option("Concertina", &actionLoadConcertina));
  sPages.back().mOptions.push_back(Option("Piano", &actionLoadPiano));
  sPages.back().mOptions.push_back(Option("BandoPiano", &actionLoadBandoPiano));
  sPages.back().mOptions.push_back(Option("Reset", &actionResetSettings));

  sPages.push_back(Page(Page::TYPE_PLAYING_NOTES, "Playing", { Option(&actionToggleDisplay) }));

  sPages.push_back(Page(Page::TYPE_PLAYING_STAFF, "", { Option(&actionToggleDisplay) }));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Options", {}));
  sPages.back().mOptions.push_back(Option("Zero", &actionResetBellows));
  sPages.back().mOptions.push_back(Option("Offset", &gSettings.zeroLoadOffset, -1, 1, 1));
  sPages.back().mOptions.push_back(Option("Transpose", &gSettings.transpose, -12, 12, 1));
  sPages.back().mOptions.push_back(Option("Key", &gSettings.accidentalKey, gKeyNames, NUM_KEYS));
  sPages.back().mOptions.push_back(Option("Stereo", &gSettings.stereo, -100, 100, 5, false));
  sPages.back().mOptions.push_back(Option("Balance", &gSettings.balance, -100, 100, 5, false));

  sPages.back().mOptions.push_back(Option("Metronome", &actionToggleMetronome));
  sPages.back().mOptions.push_back(Option("Beats/min", &gSettings.metronomeBeatsPerMinute, 20, 200, 1, false));
  sPages.back().mOptions.push_back(Option("Beats/bar", &gSettings.metronomeBeatsPerBar, 1, 10, 1, false));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Left", {}));
  sPages.back().mOptions.push_back(Option("Expression", &gSettings.expressions[LEFT], gExpressionNames, EXPRESSION_NUM));
  sPages.back().mOptions.push_back(Option("Max vel", &gSettings.maxVelocity[LEFT], 0, 127, 1, false));
  sPages.back().mOptions.push_back(Option("Off vel", &gSettings.noteOffVelocity[LEFT], 0, 127, 1, false));
  sPages.back().mOptions.push_back(Option("Octave", &gSettings.octave[LEFT], -2, 2, 1));
  sPages.back().mOptions.push_back(Option("Instrument", &gSettings.midiInstruments[LEFT], -1, 127, 1, true));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Right", {}));
  sPages.back().mOptions.push_back(Option("Expression", &gSettings.expressions[RIGHT], gExpressionNames, EXPRESSION_NUM));
  sPages.back().mOptions.push_back(Option("Max vel", &gSettings.maxVelocity[RIGHT], 0, 127, 1, false));
  sPages.back().mOptions.push_back(Option("Off vel", &gSettings.noteOffVelocity[RIGHT], 0, 127, 1, false));
  sPages.back().mOptions.push_back(Option("Octave", &gSettings.octave[RIGHT], -2, 2, 1));
  sPages.back().mOptions.push_back(Option("Instrument", &gSettings.midiInstruments[RIGHT], -1, 127, 1, true));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Bellows", {}));
  sPages.back().mOptions.push_back(Option("Bellows", &gSettings.forceBellows, sForceBellowsStrings, 3));
  sPages.back().mOptions.push_back(Option("Zero", &actionResetBellows));
  sPages.back().mOptions.push_back(Option("Offset", &gSettings.zeroLoadOffset, -1, 1, 1));
  sPages.back().mOptions.push_back(Option("Dead zone", &gSettings.deadzone, 0, 50, 1, false));
  sPages.back().mOptions.push_back(Option("Attack 25%", &gSettings.attack25, 0, 100, 5, false));
  sPages.back().mOptions.push_back(Option("Attack 50%", &gSettings.attack50, 0, 100, 5, false));
  sPages.back().mOptions.push_back(Option("Attack 75%", &gSettings.attack75, 0, 100, 5, false));
  sPages.back().mOptions.push_back(Option("Press gain", &gSettings.pressureGain, 10, 200, 10, false));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Metronome", {}));
  sPages.back().mOptions.push_back(Option("Volume", &gSettings.metronomeVolume, 0, 100, 5, false));
  sPages.back().mOptions.push_back(Option("Note 1", &gSettings.metronomeMidiNotePrimary, 1, 127, 1, true));
  sPages.back().mOptions.push_back(Option("Note 2", &gSettings.metronomeMidiNoteSecondary, 1, 127, 1, true));
  sPages.back().mOptions.push_back(Option("Instrument", &gSettings.metronomeMidiInstrument, 0, 127, 1, true));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Misc", {}));
  sPages.back().mOptions.push_back(Option("Layout", &gSettings.noteLayout, gNoteLayoutNames, NOTELAYOUTTYPE_NUM));
  sPages.back().mOptions.push_back(Option("Notes", &gSettings.accidentalPreference, gAccidentalPreferenceNames, 3));
  sPages.back().mOptions.push_back(Option("Debounce", &gSettings.debounceTime, 0, 50, 1));
  sPages.back().mOptions.push_back(Option("Brightness", &gSettings.menuBrightness, 4, 0xf, 1, false, &forceMenuRefresh));
  sPages.back().mOptions.push_back(Option("Note disp.", &gSettings.noteDisplay, gNoteDisplayNames, NOTE_DISPLAY_NUM));
  sPages.back().mOptions.push_back(Option("Toggle FPS", &actionShowFPS));

  // This isn't useful at the moment - may kill it
  // sPages.push_back(Page(Page::TYPE_STATUS, "Status", { Option(Option(&actionToggleDisplay)) }));

  gSettings.menuPageIndex = std::clamp(gSettings.menuPageIndex, 0, (int)(sPages.size() - 1));

  display.clearDisplay();
  display.display();
  display.setTextColor(gSettings.menuBrightness, 0x0);
  display.setTextWrap(false);

#if 1
  scrollInText(0, 0, "Bandon.ino", 3);
  scrollInText(0, 8, "Danny Chapman", 3);
  delay(200);
#endif

  display.clearDisplay();
  display.display();
  forceMenuRefresh();
  ;
}

static std::vector<int> sLastPlayingNotes[2];

//====================================================================================================
int convertToScreenY(int y) {
  return 127 - y;
}

int lowestY = 0 + sCharHeight * 2;
int highestY = 85 + sCharHeight * 2;
int minMidi[2] = { NOTE(CN, 2), NOTE(AN, 3) };
int maxMidi[2] = { NOTE(AN, 4), NOTE(BN, 6) };

//====================================================================================================
void displayPlayingNotes(int side) {
  byte* playingNotes = gBigState.mPlayingNotes[side];
  std::vector<int>& lastNotes = sLastPlayingNotes[side];

  static std::vector<int> notes;
  notes.clear();
  for (int i = gSettings.midiMin; i <= gSettings.midiMax; ++i) {
    if (playingNotes[i]) {
      notes.push_back(i);
    }
  }
  if (notes == lastNotes)
    return;

  display.setTextSize(2);
  if (gSettings.noteDisplay == NOTE_DISPLAY_PLACED) {
    // Place notes according to their pitch
    const int offset = 0;
    int col = side ? 128 - offset - 3 * 2 * sCharWidth : offset;
    int pushDelta = side ? -sCharWidth * 3 * 2 : sCharWidth * 3 * 2;
    // Clear any previous notes
    for (int note : lastNotes) {
      float frac = (note - minMidi[side]) / float(maxMidi[side] - minMidi[side]);
      int y = lowestY + frac * (highestY - lowestY);
      if (side)
        display.setCursor(col + pushDelta, convertToScreenY(y));
      else
        display.setCursor(col, convertToScreenY(y));
      display.printf("      ");
    }
    // Display new notes, but don't write to the background in case of some remaining overlap
    display.setTextColor(gSettings.menuBrightness, gSettings.menuBrightness);
    int prevY = -1000;
    bool prevPushed = false;
    for (int note : notes) {
      float frac = (note - minMidi[side]) / float(maxMidi[side] - minMidi[side]);
      int y = lowestY + frac * (highestY - lowestY);
      if (y < prevY + 2 * sCharHeight && !prevPushed) {
        prevPushed = true;
        display.setCursor(col + pushDelta, convertToScreenY(y));
      } else {
        display.setCursor(col, convertToScreenY(y));
        prevPushed = false;
      }
      NoteInfo noteInfo = getNoteInfo(note, CLEF_TREBLE, gSettings.accidentalPreference, gSettings.accidentalKey);
      display.printf("%-3s", noteInfo.mName);
      prevY = y;
    }
    display.setTextColor(gSettings.menuBrightness, 0x0);
  } else {
    // Display notes by stacking them - this is OK, but notes will jump around and it's not
    // always obvious whether it's high or low
    const int offset = 16;
    int col = side ? 127 - offset - 3 * 2 * sCharWidth : offset;
    int row = 5;
    size_t num = std::max(notes.size(), lastNotes.size());
    for (size_t i = 0; i < num; ++i, --row) {
      if (row <= 0)
        break;
      display.setCursor(col, sPageY + row * sCharHeight * 2);
      if (i < notes.size()) {
        NoteInfo noteInfo = getNoteInfo(notes[i], CLEF_TREBLE, gSettings.accidentalPreference, gSettings.accidentalKey);
        display.printf("%-3s", noteInfo.mName);
      } else {
        display.printf("   ");
      }
    }
  }
  display.display();
  display.setTextSize(1);

  lastNotes.swap(notes);  // no memory copies or allocations
}

//====================================================================================================
void displayPressure() {
  display.setCursor(75, sPageY);
  static const char* bellowsIndicators[3] = { ">||<", "=||=", "<||>" };
  display.printf("%s %3.2f", bellowsIndicators[gState.mBellowsState + 1], gState.mAbsPressure);
  display.display();
}

//====================================================================================================
void displayAllPlayingNotes() {
  displayPlayingNotes(LEFT);
  displayPlayingNotes(RIGHT);
  displayPressure();
}

const int STAFF_LINE_SPACING = 8;
const int STAFF_Y_START = 32;
const int NOTE_X[2] = { 47, 110 };
const int LEDGER_X[2] = { 41, 104 };
const int LEDGER_WIDTH = 12;
const int MAX_STAFF_LINE[2] = { 7, 8 };
const int LEDGER_LINES_COLOUR = 0x8;
const int STAFF_BITMAP_COLOUR = 0xff;
const int NOTE_COLOUR = 0xff;

//====================================================================================================
// Record the last area plotted so we can quickly wipe it
struct Area {
  Area() {
    Reset();
  }

  void Reset() {
    mX0 = 127;
    mX1 = 0;
    mY0 = 127;
    mY1 = 0;
  }

  void AddPoint(int x, int y) {
    x = std::clamp(x, 0, 127);
    y = std::clamp(y, 0, 127);
    mX0 = std::min(mX0, x);
    mX1 = std::max(mX1, x);
    mY0 = std::min(mY0, y);
    mY1 = std::max(mY1, y);
  }
  bool IsValid() const {
    return mX1 > mX0 && mY1 > mY0;
  }
  uint16_t W() const {
    return 1 + (uint16_t)(mX1 - mX0);  // When x1 = X0, that's a size of 1
  }
  uint16_t H() const {
    return 1 + (uint16_t)(mY1 - mY0);
  }
  uint16_t X() const {
    return (uint16_t)mX0;
  }
  uint16_t Y() const {
    return (uint16_t)mY0;
  }

  int mX0, mY0, mX1, mY1;
};

//====================================================================================================
void drawStaffLines(int startLine, int endLine, int x, int width, uint16_t colour, Area* area = nullptr) {
  for (int i = startLine; i <= endLine; ++i) {
    int y = STAFF_Y_START + i * STAFF_LINE_SPACING;
    int screenY = convertToScreenY(y);
    display.drawFastHLine(x, screenY, width, colour);
    if (area) {
      area->AddPoint(x, screenY);
      area->AddPoint(x + width, screenY);
    }
    display.display();
  }
}

//====================================================================================================
void displayStaffPage() {
  drawStaffLines(0, 4, 0, 128, STAFF_BITMAP_COLOUR);
  display.drawBitmap(0, 0, ClefPage, 128, 128, STAFF_BITMAP_COLOUR);
  display.display();
}

//====================================================================================================
// x is in pixels. y is in numbers starting from the bottom staff (not ledger) line
void drawNote(int x, int note, uint16_t colour, Area& area) {
  // My y is the position of the bottom corner, starting from the bottom
  int y = STAFF_Y_START + (STAFF_LINE_SPACING / 2) * note + NoteHeadOffsets[1];
  int screenX = x + NoteHeadOffsets[0];
  int screenY = convertToScreenY(y) - NoteHeadSize[1];
  display.drawBitmap(screenX, screenY, NoteHeadSpace, NoteHeadSize[0], NoteHeadSize[1], colour);
  area.AddPoint(screenX, screenY);
  area.AddPoint(screenX + NoteHeadSize[0], screenY + NoteHeadSize[1]);
}

//====================================================================================================
void drawAccidental(int x, int note, int accidental, uint16_t colour, Area& area) {
  if (!accidental)
    return;
  const unsigned char* bitmap = accidental > 0 ? SharpSpace : FlatSpace;
  const uint16_t* size = accidental > 0 ? SharpSize : FlatSize;
  const int* offset = accidental > 0 ? SharpOffsets : FlatOffsets;

  int screenX = x + offset[0];
  int y = STAFF_Y_START + (STAFF_LINE_SPACING / 2) * note + offset[1] + size[1];
  int screenY = convertToScreenY(y);

  display.drawBitmap(screenX, screenY, bitmap, size[0], size[1], colour);
  area.AddPoint(screenX, screenY);
  area.AddPoint(screenX + size[0], screenY + size[1]);
}

static std::vector<int> sLastStaffNotes[2];
static Area sLastAreas[2];

//====================================================================================================
void displayPlayingStaff(int side) {
  const byte* playingNotes = gBigState.mPlayingNotes[side];
  std::vector<int>& lastNotes = sLastPlayingNotes[side];
  Area& area = sLastAreas[side];

  static std::vector<int> notes;
  notes.clear();
  for (int i = gSettings.midiMin; i <= gSettings.midiMax; ++i) {
    if (playingNotes[i]) {
      notes.push_back(i);
    }
  }
  if (notes == lastNotes)
    return;

  // Wipe and refresh the area that was previously used
  if (area.IsValid()) {
    display.fillRect(area.X(), area.Y(), area.W(), area.H(), 0);
    display.display();
    drawStaffLines(0, 4, area.X(), area.W(), STAFF_BITMAP_COLOUR);
  }

  area.Reset();

  // How to display "crunchy" chords?
  // The book "Music Notation" by Gardner Read, second edition:
  // The interval of a second... should be written with the stem between the note-heads. The higher pitch is always placed to the right.
  // If there is a crunch, then the notes should alternate left right
  bool prevPushedSideways = false;
  NoteInfo prevNoteInfo(-999, 0);
  for (size_t iNote = 0; iNote != notes.size(); ++iNote) {
    int pushOffset = 0;
    int midiNote = notes[iNote];
    NoteInfo noteInfo = getNoteInfo(midiNote, side, gSettings.accidentalPreference, gSettings.accidentalKey);
    if (prevNoteInfo.mStavePosition + 1 >= noteInfo.mStavePosition && !prevPushedSideways) {
      pushOffset = 6;
      prevPushedSideways = true;
    } else {
      prevPushedSideways = false;
    }
    drawNote(NOTE_X[side] + pushOffset, noteInfo.mStavePosition, NOTE_COLOUR, area);
    drawAccidental(NOTE_X[side] + pushOffset, noteInfo.mStavePosition, noteInfo.mAccidental, NOTE_COLOUR, area);
    display.display();
    prevNoteInfo = noteInfo;
  }

  if (!notes.empty()) {
    int lowestMidi = notes.front();
    int highestMidi = notes.back();
    NoteInfo lowestNoteInfo = getNoteInfo(lowestMidi, side, gSettings.accidentalPreference, gSettings.accidentalKey);
    NoteInfo highestNoteInfo = getNoteInfo(highestMidi, side, gSettings.accidentalPreference, gSettings.accidentalKey);
    int requiredLines = -lowestNoteInfo.mStavePosition / 2;
    if (requiredLines > 0) {
      drawStaffLines(-requiredLines, -1, LEDGER_X[side], LEDGER_WIDTH, LEDGER_LINES_COLOUR, &area);
    }
    requiredLines = (highestNoteInfo.mStavePosition / 2);
    if (requiredLines > 4) {
      drawStaffLines(5, requiredLines, LEDGER_X[side], LEDGER_WIDTH, LEDGER_LINES_COLOUR, &area);
    }
  }

  lastNotes.swap(notes);  // no memory copies or allocations
}

//====================================================================================================
void displayPlayingStaffs() {
  displayPlayingStaff(LEFT);
  displayPlayingStaff(RIGHT);
  displayPressure();
}

//====================================================================================================
void displayStatus(const State& gState) {
  display.setCursor(0, sPageY + 1 * sCharHeight);
  display.printf("Abs pressure %3.2f\n", gState.mAbsPressure);
  display.printf("Mod pressure %3.2f\n", gState.mModifiedPressure);
  display.printf("FPS %3.1f\n", sAverageFPS);
  display.printf("Worst FPS %3.1f\n", sWorstFPS);
  display.display();
}

//====================================================================================================
void displayTitle(const char* title) {
  display.setFont(sPageTitleFont);
  display.setCursor(0, sPageTitleFont->yAdvance);
  display.print(title);
  display.setFont(nullptr);
}

//====================================================================================================
void displayOption(int pageIndex, int optionIndex, int row, bool highlightLeft, bool highlightRight) {
  if (pageIndex < 0 || pageIndex >= (int)sPages.size())
    return;
  const Page& page = sPages[pageIndex];
  if (optionIndex < 0 || optionIndex >= (int)page.mOptions.size())
    return;
  const Option& option = page.mOptions[optionIndex];

  const char* iconLeft = highlightLeft ? "*" : " ";
  const char* iconRight = highlightRight ? "*" : " ";

  if (highlightLeft || highlightRight)
    display.setTextColor(0xf, 0x0);

  display.setCursor(0, sPageY + row * sCharHeight);
  if (option.mType == Option::TYPE_ACTION) {
    display.printf("%s%-10s %8s%s", iconLeft, option.mName, "<click>", iconLeft);
  } else {
    if (option.mIntValue) {
      if (option.mValueStrings) {
        display.printf("%s%-10s %8s%s", iconLeft, option.mName, option.mValueStrings[*option.mIntValue], iconRight);
      } else {
        display.printf("%s%-10s %8d%s", iconLeft, option.mName, *option.mIntValue, iconRight);
      }
    } else if (option.mFloatValue) {
      display.printf("%s%-10s %8.2f%s", iconLeft, option.mName, *option.mFloatValue, iconRight);
    }
  }

  display.setTextColor(gSettings.menuBrightness, 0x0);
}

//====================================================================================================
Page& currentPage() {
  return sPages[gSettings.menuPageIndex];
}

//====================================================================================================
Option& currentOption() {
  return currentPage().mOptions[sCurrentOptionIndex];
}

//====================================================================================================
void updateFrameTiming() {
  static uint32_t lastMicros = micros();
  int frameMicros = micros() - lastMicros;
  lastMicros += frameMicros;

  static int lastFPSTime = gState.mLoopStartTimeMillis;
  static int framesSinceLast = 0;
  static int worstFrameTimeMicros = 0;
  int timeSinceFPS = gState.mLoopStartTimeMillis - lastFPSTime;
  if (timeSinceFPS > 1000) {
    sWorstFPS = 1000000.0f / worstFrameTimeMicros;
    sAverageFPS = 1000.0f * framesSinceLast / timeSinceFPS;
    lastFPSTime = gState.mLoopStartTimeMillis;
    // Serial.printf("FPS %3.1f\n", sAverageFPS);
    // Serial.printf("Worst FPS %3.1f\n", sWorstFPS);
    // Serial.printf("Worst frame %3.1f\n", worstFrameTimeMicros / 1000.0f);
    framesSinceLast = 0;
    worstFrameTimeMicros = 0;
  } else {
    ++framesSinceLast;
    worstFrameTimeMicros = std::max(worstFrameTimeMicros, frameMicros);
  }
}

//====================================================================================================
template<typename T>
T wrap(const T& value, const T& minValue, const T& maxValue) {
  const T delta = maxValue - minValue;
  if (value < minValue)
    return value + (delta + 1);
  if (value > maxValue)
    return value - (delta + 1);
  return value;
}

//====================================================================================================
void updateMenu() {
  updateFrameTiming();

  int deltaRotaryEncoder = gState.mRotaryEncoderPosition - gPrevState.mRotaryEncoderPosition;

  bool toggledOptionValue = false;
  bool changedOption = sForceMenuRefresh;
  bool changedValue = sForceMenuRefresh;
  sForceMenuRefresh = false;

  // Detect click
  if (gState.mRotaryEncoderPressed && !gPrevState.mRotaryEncoderPressed) {
    if (currentOption().mType == Option::TYPE_OPTION) {
      sAdjustOption = !sAdjustOption;
      toggledOptionValue = true;
    }
    if (currentOption().mAction) {
      currentOption().mAction();
    } else {
      // If there's no action, then force the display to come on in case we end
      // up in an options list with the display off!
      enableDisplay();
    }
  }

  if (deltaRotaryEncoder)
    enableDisplay();

  // If display is disabled, then only respond to the click to wake it up
  if (!gSettings.menuDisplayEnabled)
    return;

  int origPageIndex = gSettings.menuPageIndex;
  if (sAdjustOption) {
    if (deltaRotaryEncoder) {
      changedOption = true;
      sCurrentOptionIndex += deltaRotaryEncoder;

      if (sCurrentOptionIndex >= (int)currentPage().mOptions.size()) {
        // Jump to next page
        if (gSettings.menuPageIndex + 1 < (int)sPages.size()) {
          ++gSettings.menuPageIndex;
          sCurrentOptionIndex = 0;
        } else {
          sCurrentOptionIndex = currentPage().mOptions.size() - 1;
        }
      }
      if (sCurrentOptionIndex < 0) {
        // Jump to previous page
        if (gSettings.menuPageIndex - 1 >= 0) {
          --gSettings.menuPageIndex;
          sCurrentOptionIndex = sPages[gSettings.menuPageIndex].mOptions.size() - 1;
        } else {
          sCurrentOptionIndex = 0;
        }
      }
    }
  } else {  // adjustOption
    if (deltaRotaryEncoder) {
      Option& option = currentOption();
      if (option.mType == Option::TYPE_OPTION) {
        changedValue = true;
        if (option.mIntValue) {
          *option.mIntValue += option.mIntDeltaValue * deltaRotaryEncoder;
          if (option.mWrap)
            *option.mIntValue = wrap(*option.mIntValue, option.mIntMinValue, option.mIntMaxValue);
          else
            *option.mIntValue = std::clamp(*option.mIntValue, option.mIntMinValue, option.mIntMaxValue);
        } else if (option.mFloatValue) {
          *option.mFloatValue += option.mFloatDeltaValue * deltaRotaryEncoder;
          if (option.mWrap)
            *option.mFloatValue = wrap(*option.mFloatValue, option.mFloatMinValue, option.mFloatMaxValue);
          else
            *option.mFloatValue = std::clamp(*option.mFloatValue, option.mFloatMinValue, option.mFloatMaxValue);
        }
        if (currentOption().mAction) {
          currentOption().mAction();
        }
      }
    }
  }

  if (gSettings.menuPageIndex != origPageIndex)
    saveSettings();

  // If the page has changed, or we require a refresh, then display the page title
  if (gSettings.menuPageIndex != sPreviousPageIndex || sCurrentOptionIndex != sPreviousOptionIndex) {
    display.setTextColor(gSettings.menuBrightness, 0x0);
    display.clearDisplay();

    const Page& page = currentPage();
    displayTitle(page.mTitle);
    if (page.mType == Page::TYPE_PLAYING_STAFF) {
      displayStaffPage();
    }
    sSplashTime = millis();
    display.display();
    sPreviousOptionIndex = sCurrentOptionIndex;
    sPreviousPageIndex = gSettings.menuPageIndex;
  }

  // Now handle the live updating info
  const Page& page = currentPage();
  if (page.mType == Page::TYPE_SPLASH) {
    uint32_t elapsedTime = millis() - sSplashTime;
    if (elapsedTime > SPLASH_DURATION && gSettings.menuDisplayEnabled) {
      gSettings.menuDisplayEnabled = false;
      display.clearDisplay();
      display.display();
    }
    if (gSettings.showFPS) {
      overlayFPS();
      display.display();
    }
  } else if (page.mType == Page::TYPE_PLAYING_NOTES) {
    displayAllPlayingNotes();
    if (gSettings.showFPS)
      overlayFPS();
    display.display();
  } else if (page.mType == Page::TYPE_PLAYING_STAFF) {
    displayPlayingStaffs();
    if (gSettings.showFPS)
      overlayFPS();
    display.display();
  } else if (page.mType == Page::TYPE_STATUS) {
    displayStatus(gState);
    if (gSettings.showFPS)
      overlayFPS();
    display.display();
  } else if (changedValue || changedOption || toggledOptionValue) {
    int maxLine = 10;
    int offset = std::max(0, sCurrentOptionIndex - maxLine);
    int numOptions = (int)currentPage().mOptions.size();
    for (int iOption = 0; iOption != numOptions; ++iOption) {
      int line = iOption - offset;
      if (line >= 0 && line <= maxLine) {
        if (iOption == sCurrentOptionIndex)
          displayOption(gSettings.menuPageIndex, iOption, line + 2, sAdjustOption, !sAdjustOption);
        else
          displayOption(gSettings.menuPageIndex, iOption, line + 2, false, false);
      }
    }
    if (strcmp(page.mTitle, "Options") == 0 || strcmp(page.mTitle, "Bellows") == 0)
      displayPressure();
    if (gSettings.showFPS)
      overlayFPS();
    display.display();
  } else {
    if (strcmp(page.mTitle, "Options") == 0 || strcmp(page.mTitle, "Bellows") == 0)
      displayPressure();
    if (gSettings.showFPS) {
      overlayFPS();
      display.display();
    }
  }
}
