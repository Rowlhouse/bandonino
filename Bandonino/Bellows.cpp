#include "Bellows.h"
#include "State.h"
#include "Settings.h"

// https://github.com/bogde/HX711
#include <HX711.h>

//====================================================================================================
HX711 loadcell;
const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;

//====================================================================================================
void initBellows() {
  // Initialise the loadcell
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  while (!loadcell.is_ready()) {
  }
  //Zero scale
  if (gSettings.zeroLoadReading == LONG_MAX) {
    zeroBellows();
  } else {
    Serial.printf("Re-using zero bellows reading %d\n", gSettings.zeroLoadReading);
  }
  gState.mPressure = 0;
}

//====================================================================================================
void zeroBellows() {
  while (!loadcell.is_ready()) {
  }
  gSettings.zeroLoadReading = loadcell.read();
  Serial.printf("Zero bellows reading measured as %d\n", gSettings.zeroLoadReading);
  gSettings.writeToCard();
}

//====================================================================================================
void updateBellows() {
  while (!loadcell.is_ready()) {
  }
  const float loadScale = 500000.0f;
  gState.mLoadReading = loadcell.read();
  gSettings.zeroLoadReading -= gSettings.zeroLoadOffset * loadScale / 100;
  gSettings.zeroLoadOffset = 0;
  gState.mPressure = -((gState.mLoadReading - gSettings.zeroLoadReading) * (gSettings.pressureGain / 100.0f)) / 500000.0f;
}
