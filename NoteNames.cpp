#include "NoteNames.h"

#include <Wire.h>

// See https://newt.phys.unsw.edu.au/jw/graphics/notesinvert.GIF

const char* midiNoteNames[] = {
  "C0",  // 0
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

  "C1",  // 12
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

  "C2",  // 24 - Cello C
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

  "C3",  // 36 - Viola C
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

  "C4",  // 48 - middle C
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

  "C5",  // 60
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

  "C6",  // 72
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

  "C7",  // 84
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

  "C8",  // 96
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

  "C9",  // 108
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

  "C10",  // 120
  "C#10",
  "D10",
  "Eb10",
  "E10",
  "F10",
  "F#10",
  "G10",
  "G#10",
  "A10",
  "Bb10",
  "B10"
};

void getNoteOffset(int note, int& offset, int& accidental)
{
  switch (note) {
    case 0:
      offset = 0; accidental = 0;  break; // C
    case 1:
      offset = 0; accidental = 1;  break; // C#
    case 2:
      offset = 1; accidental = 0;  break; // D
    case 3:
      offset = 2; accidental = -1;  break; // Eb
    case 4:
      offset = 2; accidental = 0;  break; // E
    case 5:
      offset = 3; accidental = 0;  break; // F
    case 6:
      offset = 3; accidental = 1;  break; // F#
    case 7:
      offset = 4; accidental = 0;  break; // G
    case 8:
      offset = 4; accidental = 1;  break; // G#
    case 9:
      offset = 5; accidental = 0;  break; // A
    case 10:
      offset = 6; accidental = -1;  break; // Bb
    case 11:
      offset = 6; accidental = 0;  break; // B
    default:
    break;
  }
}

//====================================================================================================
NoteInfo getNoteInfo(int midi, int clef) {
  NoteInfo result;
  int refMidi = (clef == CLEF_BASS) ? 43 : 64;
  int refNote = refMidi % 12;
  int refOctave = refMidi / 12;
  int refOffset = 0;
  int refAccidental = 0;
  getNoteOffset(refNote, refOffset, refAccidental);
  int refHeight = refOffset + refOctave * 7;

  Serial.printf("refMidi = %d\n", refMidi);

  Serial.printf("Ref note %d octave %d height %d\n", refNote, refOctave, refHeight);

  int note = midi % 12;
  int octave = midi / 12;
  int offset = 0;
  getNoteOffset(note, offset, result.mAccidental);
  int height = offset + octave * 7;

  Serial.printf("Note %d octave %d height %d\n", note, octave, height);

  result.mStavePosition = height - refHeight;
  return result;
}
