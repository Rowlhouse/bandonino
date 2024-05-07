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

// See https://newt.phys.unsw.edu.au/jw/graphics/notesinvert.GIF for mapping of
// midi to note names

// Get the row based on the accidental (add on 5)
// Going up from the key note, the note changes to have nice sharps and flats should be:
// 2, 1, 2, 2, 2, 1, 2
const char* sNoteNames[NUM_KEYS][12] = {
  { "C", "Db", "D", "Eb", "Fb", "F", "Gb", "G", "Ab", "A", "Bb", "Cb" },  // -5 Db
  { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "Cb" },   // -4 Ab
  { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" },    // -3 Eb
  { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" },    // -2 Bb
  { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" },    // -1 F

  { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B" },  // 0  C

  { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Bb", "B" },     // 1  G
  { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },     // 2  D
  { "C", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "B" },    // 3  A
  { "B#", "C#", "D", "D#", "E", "E#", "F#", "G", "G#", "A", "A#", "B" },   // 4  E
  { "B#", "C#", "D", "D#", "E", "E#", "F#", "F+", "G#", "A", "A#", "B" },  // 5  B
};

struct NA {
  NA(byte note, byte accidental)
    : mNote(note), mAccidental(accidental) {}
  int8_t mNote;
  int8_t mAccidental;
};

const NA sNoteAccidentals[NUM_KEYS][12] = {
  { NA(0, 0), NA(1, -1), NA(1, 0), NA(2, -1), NA(3, -1), NA(3, 0), NA(4, -1), NA(4, 0), NA(5, -1), NA(5, 0), NA(6, -1), NA(7, -1) },  // -5 Db
  { NA(0, 0), NA(1, -1), NA(1, 0), NA(2, -1), NA(2, 0), NA(3, 0), NA(4, -1), NA(4, 0), NA(5, -1), NA(5, 0), NA(6, -1), NA(7, -1) },   // -4 Ab
  { NA(0, 0), NA(1, -1), NA(1, 0), NA(2, -1), NA(2, 0), NA(3, 0), NA(4, -1), NA(4, 0), NA(5, -1), NA(5, 0), NA(6, -1), NA(6, 0) },    // -3 Eb
  { NA(0, 0), NA(1, -1), NA(1, 0), NA(2, -1), NA(2, 0), NA(3, 0), NA(3, 1), NA(4, 0), NA(5, -1), NA(5, 0), NA(6, -1), NA(6, 0) },     // -2 Bb
  { NA(0, 0), NA(0, 1), NA(1, 0), NA(2, -1), NA(2, 0), NA(3, 0), NA(3, 1), NA(4, 0), NA(5, -1), NA(5, 0), NA(6, -1), NA(6, 0) },      // -1 F
  { NA(0, 0), NA(0, 1), NA(1, 0), NA(2, -1), NA(2, 0), NA(3, 0), NA(3, 1), NA(4, 0), NA(4, 1), NA(5, 0), NA(6, -1), NA(6, 0) },       // C
  { NA(0, 0), NA(0, 1), NA(1, 0), NA(1, 1), NA(2, 0), NA(3, 0), NA(3, 1), NA(4, 0), NA(4, 1), NA(5, 0), NA(6, -1), NA(6, 0) },        // 1 G
  { NA(0, 0), NA(0, 1), NA(1, 0), NA(1, 1), NA(2, 0), NA(3, 0), NA(3, 1), NA(4, 0), NA(4, 1), NA(5, 0), NA(5, 1), NA(6, 0) },         // 2 D
  { NA(0, 0), NA(0, 1), NA(1, 0), NA(1, 1), NA(2, 0), NA(2, 1), NA(3, 1), NA(4, 0), NA(4, 1), NA(5, 0), NA(5, 1), NA(6, 0) },         // 3 A
  { NA(-1, 1), NA(0, 1), NA(1, 0), NA(1, 1), NA(2, 0), NA(2, 1), NA(3, 1), NA(4, 0), NA(4, 1), NA(5, 0), NA(5, 1), NA(6, 0) },        // 4 E
  { NA(-1, 1), NA(0, 1), NA(1, 0), NA(1, 1), NA(2, 0), NA(2, 1), NA(3, 1), NA(3, 2), NA(4, 1), NA(5, 0), NA(5, 1), NA(6, 0) }         // 5 B
};


//====================================================================================================
// This returns the offset from C for a note, adjusted so the accidentals are nice
// for it having the key passed in
NA getNoteAccidentalForKey(int note, int accidentalKey) {
  accidentalKey = std::clamp(accidentalKey, 0, NUM_KEYS);
  return sNoteAccidentals[accidentalKey][note];
}

//====================================================================================================
NoteInfo getNoteInfo(int midi, int clef, int accidentalPreference, int accidentalKey) {

  if (accidentalPreference == ACCIDENTAL_PREFERENCE_SHARP) {
    accidentalKey = KEY_OFFSET + 2;
  } else if (accidentalPreference == ACCIDENTAL_PREFERENCE_FLAT) {
    accidentalKey = KEY_OFFSET - 3;
  }

  NoteInfo result;
  int refMidi = (clef == CLEF_BASS) ? 43 : 64;  // G2 or E4
  int refNote = refMidi % 12;
  int refOctave = refMidi / 12;
  NA naRef = getNoteAccidentalForKey(refNote, KEY_OFFSET);
  int refHeight = naRef.mNote + refOctave * 7;

  result.mNote = midi % 12;
  result.mOctave = midi / 12;
  NA na = getNoteAccidentalForKey(result.mNote, accidentalKey);
  int height = na.mNote + result.mOctave * 7;
  result.mAccidental = na.mAccidental;

  // Serial.printf("note = %d octave = %d, accidental = %d\n",
  //               result.mNote, result.mOctave, result.mAccidental);

  result.mStavePosition = height - refHeight;

  int wrappedNote = result.mNote;
  int wrappedOctave = result.mOctave;
  if (wrappedNote >= 12) {
    wrappedNote -= 12;
    ++wrappedOctave;
  } else if (wrappedNote < 0) {
    wrappedNote += 12;
    --wrappedOctave;
  }
  int row = std::clamp(accidentalKey, 0, NUM_KEYS);
  snprintf(result.mName, 4, "%2s%1d", sNoteNames[row][wrappedNote], wrappedOctave);

  return result;
}

