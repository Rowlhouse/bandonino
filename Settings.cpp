#include "Settings.h"
#include "NoteLayouts.h"
#include "PinInputs.h"
#include "State.h"

#include <algorithm>

Settings settings;

//====================================================================================================
void Settings::updateMIDIRange() {
  midiMin = 127;
  midiMax = 0;
  if (bigState.noteLayoutLeftOpen) {
    for (int i = 0; i != PinInputs::keyCountLeft; ++i) {
      midiMax = std::max(bigState.noteLayoutLeftOpen[i], midiMax);
      midiMin = std::min(bigState.noteLayoutLeftOpen[i], midiMin);
      midiMax = std::max(bigState.noteLayoutLeftClose[i], midiMax);
      midiMin = std::min(bigState.noteLayoutLeftClose[i], midiMin);
    }
  }
  if (bigState.noteLayoutRightOpen) {
    for (int i = 0; i != PinInputs::keyCountRight; ++i) {
      midiMax = std::max(bigState.noteLayoutRightOpen[i], midiMax);
      midiMin = std::min(bigState.noteLayoutRightOpen[i], midiMin);
      midiMax = std::max(bigState.noteLayoutRightClose[i], midiMax);
      midiMin = std::min(bigState.noteLayoutRightClose[i], midiMin);
    }
  }
}
