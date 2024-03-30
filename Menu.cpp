#include <algorithm>
#include "core_pins.h"
// SSD1306Ascii v1.3.5
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#include "Menu.h"
#include "Settings.h"
#include "State.h"
#include "NoteNames.h"

// OLED I2C address and library configuration
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

enum OptionType {
  OPTION_TYPE_INT,
  OPTION_TYPE_PERCENT,
  OPTION_TYPE_LAYOUT,
  OPTION_TYPE_MAX
};

struct Option {
  Option(const char* name, int* value, int minValue, int maxValue, int deltaValue)
    : mName(name), mIntValue(value), mIntMinValue(minValue), mIntMaxValue(maxValue), mIntDeltaValue(deltaValue) {}

  Option(const char* name, int* value, const char** valueStrings, int numStrings)
    : mName(name), mValueStrings(valueStrings), mIntValue(value), mIntMinValue(0), mIntMaxValue(numStrings-1), mIntDeltaValue(1) {}

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
  Option("Press gain", &settings.pressureGain, 0.0f, 2.0f, 0.1f),
  Option("Pan left",  &settings.panLeft, -100, 100, 5),
  Option("Pan right", &settings.panRight, -100, 100, 5),
  Option("Bellows", &settings.forceBellows, sForceBellowsStrings, 3),
  Option("Attack 25%", &settings.attack25, 0.0f, 1.0f, 0.02f),
  Option("Attack 50%", &settings.attack50, 0.0f, 1.0f, 0.02f),
  Option("Attack 75%", &settings.attack75, 0.0f, 1.0f, 0.02f),
};

const int NUM_OPTIONS = sizeof(sOptions) / sizeof(sOptions[0]);

static int sPreviousOption = -100;
static int sCurrentOption = PAGE_OPTIONS_PLAYING_NOTES;

// toggle between adjusting option vs value by clicking
static bool sAdjustOption = true;

// fonts are width by height
const uint8_t* sPageTitleFont = Arial14;
const uint8_t* sNormalFont = Adafruit5x7;
const uint8_t* sJumboFont = Arial14;

uint32_t sSplashTime = 0;  // When splash was triggered
const uint32_t SPLASH_DURATION = 2000;

bool sDisplayEnabled = true;

//====================================================================================================
void scrollInText(int col, int row, const char* text, int ms) {
  oled.setFont(sNormalFont);
  int charWidth = sNormalFont[FONT_WIDTH];

  const int textLen = strlen(text);
  for (int i = textLen * charWidth; i-- != 0;) {
    oled.setCursor(col * charWidth + i, row);
    oled.print(text);
    delay(ms);
  }
}

//====================================================================================================
// Display is 128x64 - so 16x8 characters
void initMenu() {
  Wire.begin();
  Wire.setClock(400000L);                    // Fast mode
  oled.begin(&Adafruit128x64, I2C_ADDRESS);  // OLED type and address

  oled.clear();
  scrollInText(0, 0, "Bandon.ino ", 3);
  scrollInText(0, 1, "Danny Chapman ", 3);
  delay(500);
  oled.clear();
}

//====================================================================================================
void displayPlayingNotes(byte playingNotes[], int row) {
  oled.setFont(sJumboFont);
  // int charWidth = sJumboFont[FONT_WIDTH];

  static char text[128];
  text[0] = 0;

  int col = 0;
  for (int i = settings.midiMin; i <= settings.midiMax; ++i) {
    if (playingNotes[i]) {
      col += sprintf(text + col, "%s ", midiNoteNames[i]);
    }
  }

  oled.setCursor(0, row);
  oled.printf(text);
  oled.clearToEOL();
}

//====================================================================================================
void displayAllPlayingNotes() {
  displayPlayingNotes(bigState.playingNotesLeft, 3);
  displayPlayingNotes(bigState.playingNotesRight, 6);

  // Also display pressure
  oled.setCursor(64, 0);
  static const char* bellowsIndicators[3] = { ">||<", "=||=", "<||>" };
  oled.printf("%s %3.2f  ", bellowsIndicators[state.bellowsOpening + 1], state.absPressure);
}

//====================================================================================================
void displayStatus(const State& state) {
  oled.setFont(sNormalFont);
  // int charWidth = sNormalFont[FONT_WIDTH];

  oled.setCursor(0, 2);
  oled.printf("Abs pressure %3.2f", state.absPressure);
  oled.setCursor(0, 3);
  oled.printf("Mod pressure %3.2f", state.modifiedPressure);
  oled.setCursor(0, 4);
  oled.printf("FPS %4.1f", 1000.0f / (state.loopStartTimeMillis - prevState.loopStartTimeMillis));
}

//====================================================================================================
void displayTitle(const char* title) {
  oled.setFont(sPageTitleFont);
  oled.setCursor(0, 0);
  oled.print(title);
}

//====================================================================================================
void displayOption(int optionIndex, int row, bool highlightLeft, bool highlightRight) {
  if (optionIndex < 0 || optionIndex >= NUM_OPTIONS)
    return;

  oled.setFont(sNormalFont);
  // int charWidth = sNormalFont[FONT_WIDTH];

  const Option& option = sOptions[optionIndex];

  const char* iconLeft = highlightLeft ? "*" : " ";
  const char* iconRight = highlightRight ? "*" : " ";

  oled.setCursor(0, row);
  if (option.mIntValue) {
    if (option.mValueStrings) {
      oled.printf("%s%-10s %8s%s", iconLeft, option.mName, option.mValueStrings[*option.mIntValue], iconRight);
    } else {
      oled.printf("%s%-10s %8d%s", iconLeft, option.mName, *option.mIntValue, iconRight);
    }
  } else if (option.mFloatValue) {
    oled.printf("%s%-10s %8.2f%s", iconLeft, option.mName, *option.mFloatValue, iconRight);
  }
}

//====================================================================================================
void updateMenu(Settings& settings, State& state) {

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

  if (sCurrentOption != sPreviousOption) {
    if (!sDisplayEnabled) {
      sDisplayEnabled = true;
      oled.ssd1306WriteCmd(SSD1306_DISPLAYON);
    }
    oled.clear();

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
    sPreviousOption = sCurrentOption;
  }

  // Now handle the live updating info
  if (sCurrentOption == PAGE_OPTIONS_SPLASH) {
    uint32_t elapsedTime = millis() - sSplashTime;
    if (elapsedTime > SPLASH_DURATION && sDisplayEnabled) {
      sDisplayEnabled = false;
      oled.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
    }
  } else if (sCurrentOption == PAGE_OPTIONS_PLAYING_NOTES) {
    displayAllPlayingNotes();
  } else if (sCurrentOption == PAGE_OPTIONS_STATUS) {
    displayStatus(state);
  } else {
    if (changedValue || changedOption || toggledOptionValue) {
      displayOption(sCurrentOption - 2, 2, false, false);
      displayOption(sCurrentOption - 1, 3, false, false);
      displayOption(sCurrentOption, 4, sAdjustOption, !sAdjustOption);
      displayOption(sCurrentOption + 1, 5, false, false);
      displayOption(sCurrentOption + 2, 6, false, false);
      displayOption(sCurrentOption + 3, 7, false, false);
    }
  }
}
