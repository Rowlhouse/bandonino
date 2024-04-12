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
  if (!settings.metronomeEnabled) {
    sNextBeat = 1;
    sNextBeatTime = state.loopStartTimeMillis + 200;
    sNextStopTime = state.loopStartTimeMillis + 200;
    sPlaying = false;
    return;
  }

  if (settings.metronomeMidiInstrument != sPreviousInstrument)
  {
    usbMIDI.sendProgramChange(settings.metronomeMidiInstrument, settings.metronomeMidiChannel);
    sPreviousInstrument = settings.metronomeMidiInstrument;
  }

  // Menu interactions can be slow, so don't trus tthe loop start time
  uint32_t time = millis();

  // Stop any playing note if it's time
  if (time >= sNextStopTime && sPlaying) {
    usbMIDI.sendNoteOff(settings.metronomeMidiNotePrimary, 0, settings.metronomeMidiChannel);
    usbMIDI.sendNoteOff(settings.metronomeMidiNoteSecondary, 0, settings.metronomeMidiChannel);
    sPlaying = false;
  }

  if (time >= sNextBeatTime) {
    int midiNote = (sNextBeat == 1) ? settings.metronomeMidiNotePrimary : settings.metronomeMidiNoteSecondary;
    int midiVolume = convertPercentToMidi(settings.metronomeVolume);
    usbMIDI.sendControlChange(0x07, midiVolume, settings.metronomeMidiChannel);
    usbMIDI.sendNoteOn(midiNote, 127, settings.metronomeMidiChannel);
    Serial.printf("Metronome beat %d\n", midiNote);

    float beatPeriod = 60.0f / settings.metronomeBeatsPerMinute;
    sNextBeatTime = time + (uint32_t)(beatPeriod * 1000);
    sNextStopTime = time + (uint32_t)(0.1f * beatPeriod * 1000);

    sNextBeat = (sNextBeat + 1) % settings.metronomeBeatsPerBar;
    sPlaying = true;
  }
}
