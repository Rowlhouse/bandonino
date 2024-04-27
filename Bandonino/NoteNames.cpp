#include <algorithm>
#include "NoteNames.h"

#include <Wire.h>

const char* gAccidentalPreferenceNames[] = {
  "Sharp",
  "Flat",
  "Natural"
};

const char* gKeyNames[NUM_KEYS] = {
  "Db", "Ab", "Eb", "Bb", "F", "C", "G", "D", "A", "E", "B"
};


// See https://newt.phys.unsw.edu.au/jw/graphics/notesinvert.GIF

const char* sMidiNoteNames[] = {
  "NaN",  // 0
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "NaN",
  "C0",  // 12
  "C#0",
  "D0",
  "Eb0",
  "E0",
  "F0",
  "F#0",
  "G0",
  "G#0",
  "A0",
  "Bb0",
  "B0",
  "C1",  // 24
  "C#1",
  "D1",
  "Eb1",
  "E1",
  "F1",
  "F#1",
  "G1",
  "G#1",
  "A1",
  "Bb1",
  "B1",
  "C2",  // 36 - Cello C
  "C#2",
  "D2",
  "Eb2",
  "E2",
  "F2",
  "F#2",
  "G2",
  "G#2",
  "A2",
  "Bb2",
  "B2",
  "C3",  // 48 - Viola C
  "C#3",
  "D3",
  "Eb3",
  "E3",
  "F3",
  "F#3",
  "G3",
  "G#3",
  "A3",
  "Bb3",
  "B3",
  "C4",  // 60 - middle C
  "C#4",
  "D4",
  "Eb4",
  "E4",
  "F4",
  "F#4",
  "G4",
  "G#4",
  "A4",
  "Bb4",
  "B4",
  "C5",  // 72
  "C#5",
  "D5",
  "Eb5",
  "E5",
  "F5",
  "F#5",
  "G5",
  "G#5",
  "A5",
  "Bb5",
  "B5",
  "C6",  // 84
  "C#6",
  "D6",
  "Eb6",
  "E6",
  "F6",
  "F#6",
  "G6",
  "G#6",
  "A6",
  "Bb6",
  "B6",
  "C7",  // 96
  "C#7",
  "D7",
  "Eb7",
  "E7",
  "F7",
  "F#7",
  "G7",
  "G#7",
  "A7",
  "Bb7",
  "B7",
  "C8",  // 108
  "C#8",
  "D8",
  "Eb8",
  "E8",
  "F8",
  "F#8",
  "G8",
  "G#8",
  "A8",
  "Bb8",
  "B8",
  "C9",  // 120
  "C#9",
  "D9",
  "Eb9",
  "E9",
  "F9",
  "F#9",
  "G9",
  "G#9",
  "A9",
  "Bb9",
  "B9",
};

// Get the row based on the accidental (add on 5)
// Going up from the key note, the note changes to have nice sharps and flats should be:
// 2, 1, 2, 2, 2, 1, 2
const char* sNoteNames[NUM_KEYS][12] = {
  { "C", "Db", "D", "Eb", "Fb", "F", "Gb", "G", "Ab", "A", "Bb", "Cb" }, // -5 Db
  { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "Cb" }, // -4 Ab
  { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }, // -3 Eb
  { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" }, // -2 Bb
  { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" }, // -1 F
  { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B" }, // 0  C
  { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Bb", "B" }, // 1  G
  { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, // 2  D
  { "C", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "B" }, // 3  A
  { "B#", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "B" }, // 4  E
  { "B#", "C#", "D", "D#", "E", "E#", "F#", "F+", "G#", "A", "A#", "B" }, // 5  B
};

const size_t NUM_MIDI_NOTE_NAMES = sizeof(sMidiNoteNames) / sizeof(sMidiNoteNames[0]);

void getNoteOffset(int note, int& offset, int& accidental) {
  switch (note) {
    case 0:
      offset = 0;
      accidental = 0;
      break;  // C
    case 1:
      offset = 0;
      accidental = 1;
      break;  // C#
    case 2:
      offset = 1;
      accidental = 0;
      break;  // D
    case 3:
      offset = 2;
      accidental = -1;
      break;  // Eb
    case 4:
      offset = 2;
      accidental = 0;
      break;  // E
    case 5:
      offset = 3;
      accidental = 0;
      break;  // F
    case 6:
      offset = 3;
      accidental = 1;
      break;  // F#
    case 7:
      offset = 4;
      accidental = 0;
      break;  // G
    case 8:
      offset = 4;
      accidental = 1;
      break;  // G#
    case 9:
      offset = 5;
      accidental = 0;
      break;  // A
    case 10:
      offset = 6;
      accidental = -1;
      break;  // Bb
    case 11:
      offset = 6;
      accidental = 0;
      break;  // B
    default:
      break;
  }
}

//====================================================================================================
NoteInfo getNoteInfo(int midi, int clef, int accidentalPreference, int accidentalKey) {
  NoteInfo result;
  int refMidi = (clef == CLEF_BASS) ? 43 : 64;
  int refNote = refMidi % 12;
  int refOctave = refMidi / 12;
  int refOffset = 0;
  int refAccidental = 0;
  getNoteOffset(refNote, refOffset, refAccidental);
  int refHeight = refOffset + refOctave * 7;

  int note = midi % 12;
  int octave = midi / 12;
  int offset = 0;
  getNoteOffset(note, offset, result.mAccidental);
  int height = offset + octave * 7;

  result.mStavePosition = height - refHeight;

  if (accidentalPreference == ACCIDENTAL_PREFERENCE_SHARP)
  {
    snprintf(result.mName, 4, "%2s%1d", sNoteNames[7][note], octave);
  }
  else if (accidentalPreference == ACCIDENTAL_PREFERENCE_FLAT)
  {
    snprintf(result.mName, 4, "%2s%1d", sNoteNames[2][note], octave);
  }
  else
  {
    int row = std::clamp(accidentalKey, 0, NUM_KEYS);
    snprintf(result.mName, 4, "%2s%1d", sNoteNames[row][note], octave);
  }

  // if (midi >= 0 && midi < NUM_MIDI_NOTE_NAMES) {
  //   result.mName = sMidiNoteNames[midi];
  // }

  return result;
}
