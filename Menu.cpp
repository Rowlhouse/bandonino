#include <avr/pgmspace.h>
#include <core_pins.h>

#include <algorithm>
#include <vector>

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
  typedef void (*Action)();

  enum Type {
    TYPE_OPTION,
    TYPE_ACTION,
    TYPE_NONE
  };

  Option(const char* name, int* value, int minValue, int maxValue, int deltaValue, Action action = nullptr)
    : mType(TYPE_OPTION), mName(name), mIntValue(value), mIntMinValue(minValue), mIntMaxValue(maxValue), mIntDeltaValue(deltaValue), mAction(action) {}

  Option(const char* name, int* value, const char** valueStrings, int numStrings, Action action = nullptr)
    : mType(TYPE_OPTION), mName(name), mValueStrings(valueStrings), mIntValue(value), mIntMinValue(0), mIntMaxValue(numStrings - 1), mIntDeltaValue(1), mAction(action) {}

  Option(const char* name, float* value, float minValue, float maxValue, float deltaValue, Action action = nullptr)
    : mType(TYPE_OPTION), mName(name), mFloatValue(value), mFloatMinValue(minValue), mFloatMaxValue(maxValue), mFloatDeltaValue(deltaValue), mAction(action) {}

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
static int sCurrentPageIndex = 0;

static int sPreviousOptionIndex = -1;
static int sCurrentOptionIndex = 0;

// toggle between adjusting option vs value by clicking
static bool sAdjustOption = true;

uint32_t sSplashTime = 0;  // When splash was triggered
const uint32_t SPLASH_DURATION = 2000;

static float sSmoothedFPS = 0;

bool sDisplayEnabled = true;

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
void zeroBellows() {
  Serial.println("Zero bellows");
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
  display.setTextColor(settings.menuBrightness, 0x0);

  state.zeroLoadReading = state.loadReading;
  forceMenuRefresh();
}

//====================================================================================================
void actionZeroBellows() {
  zeroBellows();
}

//====================================================================================================
void actionSaveSettings() {
  Serial.println("Save settings");
  char filename[32];
  sprintf(filename, "Settings%02d.json", settings.slot);
  if (!settings.writeToCard(filename))
    Serial.printf("Failed to write settings to %s\n", filename);
  else
    showMessage("Saved", 500);
}

//====================================================================================================
void actionResetSettings() {
  settings = Settings();
  showMessage("Reset", 500);
}

//====================================================================================================
void actionLoadSettings() {
  Serial.println("Load settings");
  char filename[32];
  sprintf(filename, "Settings%02d.json", settings.slot);
  if (!settings.readFromCard(filename))
    Serial.printf("Failed to write settings to %s\n", filename);
  else
    showMessage("Loaded", 500);
}

//====================================================================================================
void actionShowFPS() {
  settings.showFPS = !settings.showFPS;
  sForceMenuRefresh = true;
  sPreviousOptionIndex = -1;
  sPreviousPageIndex = -1;
}

//====================================================================================================
void forceMenuRefresh() {
  sForceMenuRefresh = true;
  sPreviousOptionIndex = -1;
  sPreviousPageIndex = -1;
}

//====================================================================================================
void actionToggleDisplay() {
  if (sDisplayEnabled) {
    sDisplayEnabled = false;
    display.clearDisplay();
    display.display();
  } else {
    sDisplayEnabled = true;
    forceMenuRefresh();
  }
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

  Serial.println("Loading settings from settings.json");
  if (!settings.readFromCard("settings.json"))
    Serial.println("Failed to load settings");

  // Not sure there's any merit to a blank page, since the display can be turned off by clicking
  // sPages.push_back(Page(Page::TYPE_SPLASH, "Bandon.ino", { Option() }));

  sPages.push_back(Page(Page::TYPE_PLAYING_NOTES, "Playing", { Option(&actionToggleDisplay) }));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Bellows", {}));
  sPages.back().mOptions.push_back(Option("Zero", &actionZeroBellows));
  sPages.back().mOptions.push_back(Option("Bellows", &settings.forceBellows, sForceBellowsStrings, 3));
  sPages.back().mOptions.push_back(Option("Attack 25%", &settings.attack25, 0, 100, 5));
  sPages.back().mOptions.push_back(Option("Attack 50%", &settings.attack50, 0, 100, 5));
  sPages.back().mOptions.push_back(Option("Attack 75%", &settings.attack75, 0, 100, 5));
  sPages.back().mOptions.push_back(Option("Press gain", &settings.pressureGain, 10, 200, 10));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Left", {}));
  sPages.back().mOptions.push_back(Option("Expression", &settings.expressionTypes[LEFT], gExpressionTypes, EXPRESSION_TYPE_NUM));
  sPages.back().mOptions.push_back(Option("Pan", &settings.pans[LEFT], -100, 100, 5));
  sPages.back().mOptions.push_back(Option("Volume", &settings.levels[LEFT], 0, 100, 5));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Right", {}));
  sPages.back().mOptions.push_back(Option("Expression", &settings.expressionTypes[RIGHT], gExpressionTypes, EXPRESSION_TYPE_NUM));
  sPages.back().mOptions.push_back(Option("Pan", &settings.pans[RIGHT], -100, 100, 5));
  sPages.back().mOptions.push_back(Option("Volume", &settings.levels[RIGHT], 0, 100, 5));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Options", {}));
  sPages.back().mOptions.push_back(Option("Layout", &settings.noteLayout, gNoteLayouts, NOTELAYOUTTYPE_NUM));
  sPages.back().mOptions.push_back(Option("Brightness", &settings.menuBrightness, 4, 0xf, 1, &forceMenuRefresh));
  sPages.back().mOptions.push_back(Option("Toggle FPS", &actionShowFPS));

  sPages.push_back(Page(Page::TYPE_OPTIONS, "Settings", {}));
  sPages.back().mOptions.push_back(Option("Slot", &settings.slot, 0, 9, 1));
  sPages.back().mOptions.push_back(Option("Save", &actionSaveSettings));
  sPages.back().mOptions.push_back(Option("Load", &actionLoadSettings));
  sPages.back().mOptions.push_back(Option("Reset", &actionResetSettings));

  // This isn't useful at the moment - may kill it
  sPages.push_back(Page(Page::TYPE_STATUS, "Status", { Option(Option(&actionToggleDisplay)) }));

  sCurrentPageIndex = 0;

  display.clearDisplay();
  display.display();
  display.setTextColor(settings.menuBrightness, 0x0);
  display.setTextWrap(false);

#if 1
  scrollInText(0, 0, "Bandon.ino", 3);
  scrollInText(0, 8, "Danny Chapman", 3);
  delay(200);
#endif

  display.clearDisplay();
  display.display();
}

static std::vector<int> lastNotesLeft;
static std::vector<int> lastNotesRight;

//====================================================================================================
void displayPlayingNotes(byte playingNotes[], int col, std::vector<int>& lastNotes) {

  static std::vector<int> notes;
  notes.clear();
  for (int i = settings.midiMin; i <= settings.midiMax; ++i) {
    if (playingNotes[i]) {
      notes.push_back(i);
    }
  }
  if (notes == lastNotes)
    return;

  display.setTextSize(2);
  int row = 5;
  size_t num = std::max(notes.size(), lastNotes.size());
  for (size_t i = 0; i < num; ++i, --row) {
    if (row <= 0)
      break;
    display.setCursor(col, sPageY + row * sCharHeight * 2);
    if (i < notes.size()) {
      display.printf("%-3s", midiNoteNames[notes[i]]);
    } else {
      display.printf("   ");
    }
  }
  display.display();
  display.setTextSize(1);

  lastNotes.swap(notes); // no memory copies or allocations
}

//====================================================================================================
void displayAllPlayingNotes() {
  int offset = 16;
  displayPlayingNotes(bigState.playingNotes[LEFT], offset, lastNotesLeft);
  displayPlayingNotes(bigState.playingNotes[RIGHT], 128 - offset - 3 * 2 * sCharWidth, lastNotesRight);

  // Also display pressure
  display.setCursor(80, sPageY);
  static const char* bellowsIndicators[3] = { ">||<", "=||=", "<||>" };
  display.printf("%s %3.2f", bellowsIndicators[state.bellowsOpening + 1], state.absPressure);
  display.display();
}

//====================================================================================================
void displayStatus(const State& state) {
  display.setCursor(0, sPageY + 1 * sCharHeight);
  display.printf("Abs pressure %3.2f\n", state.absPressure);
  display.printf("Mod pressure %3.2f\n", state.modifiedPressure);
  display.printf("FPS %3.1f\n", sSmoothedFPS);
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

  display.setTextColor(settings.menuBrightness, 0x0);
}

//====================================================================================================
Page& currentPage() {
  return sPages[sCurrentPageIndex];
}

//====================================================================================================
Option& currentOption() {
  return currentPage().mOptions[sCurrentOptionIndex];
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
  bool changedOption = sForceMenuRefresh;
  bool changedValue = sForceMenuRefresh;
  sForceMenuRefresh = false;

  // Detect click
  if (state.rotaryEncoderPressed && !prevState.rotaryEncoderPressed) {
    if (currentOption().mType == Option::TYPE_OPTION) {
      sAdjustOption = !sAdjustOption;
      toggledOptionValue = true;
    }
    if (currentOption().mAction) {
      currentOption().mAction();
    }
  }

  int origPageIndex = sCurrentPageIndex;
  if (sAdjustOption) {
    if (deltaRotaryEncoder) {
      changedOption = true;
      sCurrentOptionIndex += deltaRotaryEncoder;

      if (sCurrentOptionIndex >= (int)currentPage().mOptions.size()) {
        // Jump to next page
        if (sCurrentPageIndex + 1 < (int)sPages.size()) {
          ++sCurrentPageIndex;
          sCurrentOptionIndex = 0;
        } else {
          sCurrentOptionIndex = currentPage().mOptions.size() - 1;
        }
      }
      if (sCurrentOptionIndex < 0) {
        // Jump to previous page
        if (sCurrentPageIndex - 1 >= 0) {
          --sCurrentPageIndex;
          sCurrentOptionIndex = sPages[sCurrentPageIndex].mOptions.size() - 1;
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
          *option.mIntValue = std::clamp(*option.mIntValue, option.mIntMinValue, option.mIntMaxValue);
        } else if (option.mFloatValue) {
          *option.mFloatValue += option.mFloatDeltaValue * deltaRotaryEncoder;
          *option.mFloatValue = std::clamp(*option.mFloatValue, option.mFloatMinValue, option.mFloatMaxValue);
        }
        if (currentOption().mAction) {
          currentOption().mAction();
        }
      }
    }
  }

  if (currentPage().mType == Page::TYPE_PLAYING_NOTES && sCurrentPageIndex < origPageIndex) {
    // We have gone back through the options to "playing notes" - so save the settings
    Serial.println("Writing settings to settings.json");
    if (!settings.writeToCard("settings.json"))
      Serial.println("Failed to write to settings.json");
  }

  // If the page has changed, or we require a refresh, then display the page title
  if (sCurrentPageIndex != sPreviousPageIndex || sCurrentOptionIndex != sPreviousOptionIndex) {
    display.setTextColor(settings.menuBrightness, 0x0);

    if (!sDisplayEnabled) {
      sDisplayEnabled = true;
    }
    display.clearDisplay();

    const Page& page = currentPage();
    displayTitle(page.mTitle);
    sSplashTime = millis();
    display.display();
    sPreviousOptionIndex = sCurrentOptionIndex;
    sPreviousPageIndex = sCurrentPageIndex;
  }

  if (!sDisplayEnabled)
    return;

  // Now handle the live updating info
    const Page& page = currentPage();
  if (page.mType == Page::TYPE_SPLASH) {
    uint32_t elapsedTime = millis() - sSplashTime;
    if (elapsedTime > SPLASH_DURATION && sDisplayEnabled) {
      sDisplayEnabled = false;
      display.clearDisplay();
      display.display();
    }
    if (settings.showFPS) {
      overlayFPS();
      display.display();
    }
  } else if (page.mType == Page::TYPE_PLAYING_NOTES) {
    displayAllPlayingNotes();
    if (settings.showFPS)
      overlayFPS();
    display.display();
  } else if (page.mType == Page::TYPE_STATUS) {
    displayStatus(state);
    if (settings.showFPS)
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
          displayOption(sCurrentPageIndex, iOption, line + 2, sAdjustOption, !sAdjustOption);
        else
          displayOption(sCurrentPageIndex, iOption, line + 2, false, false);
      }
    }
    if (settings.showFPS)
      overlayFPS();
    display.display();
  } else {
    if (settings.showFPS) {
      overlayFPS();
      display.display();
    }
  }
}
