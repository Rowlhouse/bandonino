#ifndef NOTENAMES_H
#define NOTENAMES_H

// MIDI pitch values. 00 means unused
// Midle C is in octave 5
#define NOTE_CN 0
#define NOTE_CS 1
#define NOTE_DN 2
#define NOTE_DS 3
#define NOTE_EN 4
#define NOTE_FN 5
#define NOTE_FS 6
#define NOTE_GN 7
#define NOTE_GS 8
#define NOTE_AN 9
#define NOTE_AS 10
#define NOTE_BN 11

#define NOTE(note, octave) (octave * 12 + NOTE_##note)
#define NOTE_UNUSED 0

extern const char* midiNoteNames[];

#endif
