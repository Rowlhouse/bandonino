#include "avr/pgmspace.h"
#include <algorithm>
#include "core_pins.h"

//====================================================================================================
// 1327 128x128 Display
//====================================================================================================
#include <Adafruit_SSD1327.h>
#define I2C_ADDRESS 0x3D
#define OLED_RESET -1
Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET, 4000000);

// Note that fonts can be generated from https://oleddisplay.squix.ch/#/home
#include "Fonts/FreeSans9pt7b.h"
static const GFXfont* sPageTitleFont = &FreeSans9pt7b;
static const int sCharWidth = 5;
static const int sCharHeight = 8;

// Screen size in the defautl character size
static const int sScreenCharWidth = 128 / sCharWidth;
static const int sScreenCharHeight = 128 / sCharHeight;

// The height of the top of page contents - i.e. draw using the default font at this y value plus char height to not
// clip the page title
static const int sPageY = 16;

//====================================================================================================

#include "Menu.h"
#include "Settings.h"
#include "State.h"
#include "NoteNames.h"

static bool sForceMenuRefresh = false;

struct Option {
  Option(const char* name, int* value, int minValue, int maxValue, int deltaValue)
    : mName(name), mIntValue(value), mIntMinValue(minValue), mIntMaxValue(maxValue), mIntDeltaValue(deltaValue) {}

  Option(const char* name, int* value, const char** valueStrings, int numStrings)
    : mName(name), mValueStrings(valueStrings), mIntValue(value), mIntMinValue(0), mIntMaxValue(numStrings - 1), mIntDeltaValue(1) {}

  Option(const char* name, float* value, float minValue, float maxValue, float deltaValue)
    : mName(name), mFloatValue(value), mFloatMinValue(minValue), mFloatMaxValue(maxValue), mFloatDeltaValue(deltaValue) {}

  const char* mName;
  const char** mValueStrings = nullptr;

  int* mIntValue = nullptr;
  float* mFloatValue = nullptr;

  int mIntMinValue = 0;
  int mIntMaxValue = 0;
  int mIntDeltaValue = 0;
  float mFloatMinValue = 0;
  float mFloatMaxValue = 0;
  float mFloatDeltaValue = 0;
};

enum SelectionMode {
  SELECTION_MODE_OPTION,
  SELECTION_MODE_VALUE,
  SELECTION_MODE_MAX
};

enum PageOptions {
  PAGE_OPTIONS_PLAYING_NOTES = -1,
  PAGE_OPTIONS_STATUS = -2,
  PAGE_OPTIONS_SPLASH = -3,
  PAGE_OPTIONS_MIN = -3
};

static const char* sForceBellowsStrings[] = {
  "Sensor", "Open", "Close"
};

static Option sOptions[] = {
  Option("Layout", &settings.noteLayout, gNoteLayouts, NOTELAYOUTTYPE_NUM),
  Option("Press gain", &settings.pressureGain, 0, 200, 10),
  Option("Pan left", &settings.panLeft, -100, 100, 5),
  Option("Pan right", &settings.panRight, -100, 100, 5),
  Option("Bellows", &settings.forceBellows, sForceBellowsStrings, 3),
  Option("Attack 25%", &settings.attack25, 0, 100, 5),
  Option("Attack 50%", &settings.attack50, 0, 100, 5),
  Option("Attack 75%", &settings.attack75, 0, 100, 5),
};

const int NUM_OPTIONS = sizeof(sOptions) / sizeof(sOptions[0]);

static int sPreviousOption = -100;
static int sCurrentOption = PAGE_OPTIONS_PLAYING_NOTES;

// toggle between adjusting option vs value by clicking
static bool sAdjustOption = true;

uint32_t sSplashTime = 0;  // When splash was triggered
const uint32_t SPLASH_DURATION = 2000;

static float sSmoothedFPS = 0;

bool sDisplayEnabled = true;

//====================================================================================================
void forceMenuRefresh() {
  sForceMenuRefresh = true;
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
  display.printf("%5.1f ", sSmoothedFPS);
}

//====================================================================================================
// Display is 128x64 - so 16x8 characters
void initMenu() {
  if (!display.begin(I2C_ADDRESS))
    Serial.println("Unable to initialize OLED");

  display.clearDisplay();
  display.display();
  display.setTextColor(0xf, 0x0);
  display.setTextWrap(false);

#if 1
  scrollInText(0, 0, "Bandon.ino", 3);
  scrollInText(0, 8, "Danny Chapman", 3);
  delay(200);
#endif

  display.clearDisplay();
  display.display();
}

// Max is Letter+accidental+octave+space times number of fingers, plus some for safety!
const size_t maxTextLen = 32;
static char lastTextLeft[maxTextLen] = { 0 };
static char lastTextRight[maxTextLen] = { 0 };

//====================================================================================================
void displayPlayingNotes(byte playingNotes[], int row, char* prevText) {
  static char text[128];
  text[0] = 0;

  int col = 0;
  for (int i = settings.midiMin; i <= settings.midiMax; ++i) {
    if (playingNotes[i]) {
      col += sprintf(text + col, "%s ", midiNoteNames[i]);
    }
  }
  if (strcmp(text, prevText)) {
    // the max number of spaces needed is:
    // 64 / sCharWidth = 13
    display.setTextSize(2);
    display.setCursor(0, sPageY + row * sCharHeight);
    display.printf("%s            ", text);
    display.setTextSize(1);
    display.display();
    strcpy(prevText, text);
  }
}

//====================================================================================================
void displayAllPlayingNotes() {
  displayPlayingNotes(bigState.playingNotesLeft, 3, lastTextLeft);
  displayPlayingNotes(bigState.playingNotesRight, 6, lastTextRight);

  // Also display pressure
  display.setCursor(80, sPageY);
  static const char* bellowsIndicators[3] = { ">||<", "=||=", "<||>" };
  display.printf("%s %3.2f", bellowsIndicators[state.bellowsOpening + 1], state.absPressure);
  display.display();

  overlayFPS();
}

//====================================================================================================
void displayStatus(const State& state) {
  display.setCursor(0, sPageY + 1 * sCharHeight);
  display.printf("Abs pressure %3.2f", state.absPressure);
  display.setCursor(0, sPageY + 2 * sCharHeight);
  display.printf("Mod pressure %3.2f", state.modifiedPressure);
  display.setCursor(0, sPageY + 3 * sCharHeight);
  display.printf("FPS %4.1f ", sSmoothedFPS);
}

//====================================================================================================
void displayTitle(const char* title) {
  display.setFont(sPageTitleFont);
  display.setCursor(0, sPageTitleFont->yAdvance);
  display.print(title);
  display.setFont(nullptr);
}

//====================================================================================================
void displayOption(int optionIndex, int row, bool highlightLeft, bool highlightRight) {
  if (optionIndex < 0 || optionIndex >= NUM_OPTIONS)
    return;
  const Option& option = sOptions[optionIndex];

  const char* iconLeft = highlightLeft ? "*" : " ";
  const char* iconRight = highlightRight ? "*" : " ";

  display.setCursor(0, sPageY + row * sCharHeight);
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

//====================================================================================================
void updateMenu(Settings& settings, State& state) {
  int deltaTime = (state.loopStartTimeMillis - prevState.loopStartTimeMillis);
  if (deltaTime > 0) {
    float fps = 1000.0f / deltaTime;
    float f = 0.05f;
    sSmoothedFPS += (fps - sSmoothedFPS) * f;
  }

  int deltaRotaryEncoder = state.rotaryEncoderPosition - prevState.rotaryEncoderPosition;

  bool toggledOptionValue = false;
  bool changedOption = false;
  bool changedValue = false;

  if (state.rotaryEncoderPressed && !prevState.rotaryEncoderPressed) {
    if (sCurrentOption >= 0) {
      sAdjustOption = !sAdjustOption;
      toggledOptionValue = true;
    }
  }

  if (sAdjustOption) {
    if (deltaRotaryEncoder) {
      changedOption = true;
      sCurrentOption += deltaRotaryEncoder;

      if (sCurrentOption < PAGE_OPTIONS_MIN) {
        sCurrentOption = PAGE_OPTIONS_MIN;
      } else if (sCurrentOption >= NUM_OPTIONS) {
        sCurrentOption = NUM_OPTIONS - 1;
      }
    }
  } else if (sCurrentOption >= 0 && deltaRotaryEncoder) {
    changedValue = true;
    const Option& option = sOptions[sCurrentOption];
    if (option.mIntValue) {
      *option.mIntValue += option.mIntDeltaValue * deltaRotaryEncoder;
      *option.mIntValue = std::clamp(*option.mIntValue, option.mIntMinValue, option.mIntMaxValue);
    } else if (option.mFloatValue) {
      *option.mFloatValue += option.mFloatDeltaValue * deltaRotaryEncoder;
      *option.mFloatValue = std::clamp(*option.mFloatValue, option.mFloatMinValue, option.mFloatMaxValue);
    }
  }

  if (sCurrentOption != sPreviousOption || sForceMenuRefresh) {
    if (!sDisplayEnabled) {
      sDisplayEnabled = true;
    }
    display.clearDisplay();

    if (sCurrentOption == PAGE_OPTIONS_SPLASH) {
      displayTitle("Bandon.ino");
      sSplashTime = millis();
    } else if (sCurrentOption == PAGE_OPTIONS_PLAYING_NOTES) {
      displayTitle("Playing");
    } else if (sCurrentOption == PAGE_OPTIONS_STATUS) {
      displayTitle("Status");
    } else {
      displayTitle("Options");
    }
    display.display();
    sPreviousOption = sCurrentOption;
  }

  // Now handle the live updating info
  if (sCurrentOption == PAGE_OPTIONS_SPLASH) {
    uint32_t elapsedTime = millis() - sSplashTime;
    if (elapsedTime > SPLASH_DURATION && sDisplayEnabled) {
      sDisplayEnabled = false;
      display.clearDisplay();
      display.display();
    } else if (sDisplayEnabled) {
      overlayFPS();
      display.display();
    }
  } else if (sCurrentOption == PAGE_OPTIONS_PLAYING_NOTES) {
    displayAllPlayingNotes();
    display.display();
  } else if (sCurrentOption == PAGE_OPTIONS_STATUS) {
    displayStatus(state);
    display.display();
  } else if (changedValue || changedOption || toggledOptionValue) {
    int numLines = 10;
    int midLine = numLines / 2;
    for (int iLine = 0; iLine != numLines; ++iLine) {
      if (iLine == midLine)
        displayOption(sCurrentOption + iLine - midLine, iLine + 2, sAdjustOption, !sAdjustOption);
      else
        displayOption(sCurrentOption + iLine - midLine, iLine + 2, false, false);
    }
    overlayFPS();
    display.display();
  } else {
    overlayFPS();
    display.display();
  }

  sForceMenuRefresh = false;
}
