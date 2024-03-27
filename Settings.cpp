#include "Settings.h"
#include "NoteLayouts.h"
#include "PinInputs.h"

#include <algorithm>

void Settings::updateMIDIRange() {
  midiMin = 127;
  midiMax = 0;
  if (noteLayoutLeftOpen) {
    for (int i = 0; i != PinInputs::keyCountLeft; ++i) {
      midiMax = std::max(noteLayoutLeftOpen[i], midiMax);
      midiMin = std::min(noteLayoutLeftOpen[i], midiMin);
      midiMax = std::max(noteLayoutLeftClose[i], midiMax);
      midiMin = std::min(noteLayoutLeftClose[i], midiMin);
    }
  }
  if (noteLayoutRightOpen) {
    for (int i = 0; i != PinInputs::keyCountRight; ++i) {
      midiMax = std::max(noteLayoutRightOpen[i], midiMax);
      midiMin = std::min(noteLayoutRightOpen[i], midiMin);
      midiMax = std::max(noteLayoutRightClose[i], midiMax);
      midiMin = std::min(noteLayoutRightClose[i], midiMin);
    }
  }
}
