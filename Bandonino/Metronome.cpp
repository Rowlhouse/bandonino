#include "Metronome.h"

#include "Settings.h"
#include "State.h"

#include <Wire.h>
#include <wiring.h>

uint32_t sNextBeat = 1;
uint32_t sNextBeatTime = 0;
uint32_t sNextStopTime = 0;
bool sPlaying = false;

int sPreviousInstrument = -1;

//====================================================================================================
inline int convertFractionToMidi(float frac) {
  return std::clamp((int)(128 * frac), 0, 127);
}

//====================================================================================================
inline int convertPercentToMidi(int percent) {
  return convertFractionToMidi(percent / 100.0f);
}

//====================================================================================================
void updateMetronome() {
  if (!gSettings.metronomeEnabled) {
    sNextBeat = 1;
    sNextBeatTime = gState.mLoopStartTimeMillis + 200;
    sNextStopTime = gState.mLoopStartTimeMillis + 200;
    sPlaying = false;
    return;
  }

  if (gSettings.metronomeMidiInstrument != sPreviousInstrument) {
    if (gSettings.metronomeMidiInstrument != 0)
      usbMIDI.sendProgramChange(gSettings.metronomeMidiInstrument, gSettings.metronomeMidiChannel);
    sPreviousInstrument = gSettings.metronomeMidiInstrument;
  }

  // Menu interactions can be slow, so don't trus tthe loop start time
  uint32_t time = millis();

  // Stop any playing note if it's time
  if (time >= sNextStopTime && sPlaying) {
    usbMIDI.sendNoteOff(gSettings.metronomeMidiNotePrimary, 0, gSettings.metronomeMidiChannel);
    usbMIDI.sendNoteOff(gSettings.metronomeMidiNoteSecondary, 0, gSettings.metronomeMidiChannel);
    sPlaying = false;
    if (gSettings.metronomeLED)
      analogWrite(LED_BUILTIN, 0);
  }

  if (time >= sNextBeatTime) {
    bool isMainBeat = (sNextBeat == 1 || gSettings.metronomeBeatsPerBar == 1);
    int midiNote = isMainBeat ? gSettings.metronomeMidiNotePrimary : gSettings.metronomeMidiNoteSecondary;
    int midiVolume = convertPercentToMidi(gSettings.metronomeVolume);
    usbMIDI.sendControlChange(0x07, midiVolume, gSettings.metronomeMidiChannel);
    usbMIDI.sendNoteOn(midiNote, 127, gSettings.metronomeMidiChannel);

    float beatPeriod = 60.0f / gSettings.metronomeBeatsPerMinute;
    sNextBeatTime = time + (uint32_t)(beatPeriod * 1000);
    sNextStopTime = time + (uint32_t)(0.1f * beatPeriod * 1000);

    if (gSettings.metronomeLED)
      analogWrite(LED_BUILTIN, isMainBeat ? 256 : 32);

    sNextBeat = (sNextBeat + 1) % gSettings.metronomeBeatsPerBar;
    sPlaying = true;
  }
}
