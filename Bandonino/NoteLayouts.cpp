#include "NoteLayouts.h"

#include "NoteNames.h"
#include "PinInputs.h"
#include "Settings.h"
#include "State.h"

#include <Arduino.h>

const char* gNoteLayoutNames[] = {
  "Manoury1",
  "Manoury2",
  "Tango142",
  "Hayden1",
  "Hayden2"
};

//====================================================================================================
const int gActionKey1 = INDEX_RIGHT(0, 2);
const int gActionKey2 = INDEX_RIGHT(1, 1);

//====================================================================================================
const char* getNoteLayoutName() {
  return gNoteLayoutNames[gSettings.noteLayout];
}


// To convert octaves from "standard" bando charts:
// C = octave 3
// c = octave 4
// c1 = octave 5
// c2 = octave 6
// c3 = octave 7

// Layouts are as you'd look at it - but some notes at the end of rows may have been moved

//====================================================================================================
// Manoury layout
//====================================================================================================
const uint8_t manoury1LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE(DN, 2), NOTE(CS, 2), NOTE(CN, 2), NOTE(GS, 2), NOTE(GN, 2), NOTE(FS, 2),
  NOTE_UNUSED, NOTE(GS, 4), NOTE(FN, 2), NOTE(EN, 2), NOTE(DS, 2), NOTE(BN, 2), NOTE(AS, 2), NOTE(AN, 2),
  NOTE(AN, 4), NOTE(FN, 4), NOTE(DN, 4), NOTE(BN, 3), NOTE(GS, 3), NOTE(FN, 3), NOTE(DN, 3), NOTE_UNUSED,
  NOTE(GN, 4), NOTE(EN, 4), NOTE(CS, 4), NOTE(AS, 3), NOTE(GN, 3), NOTE(EN, 3), NOTE(CS, 3), NOTE_UNUSED,
  NOTE(FS, 4), NOTE(DS, 4), NOTE(CN, 4), NOTE(AN, 3), NOTE(FS, 3), NOTE(DS, 3), NOTE(CN, 3), NOTE_UNUSED
};

const uint8_t manoury1LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(GS, 6), NOTE(AN, 6), NOTE(AS, 6), NOTE(BN, 6),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(AN, 3), NOTE(AS, 3), NOTE(FN, 6), NOTE(FS, 6), NOTE(GN, 6),
  NOTE(DS, 4), NOTE_UNUSED, NOTE(BN, 3), NOTE(CN, 4), NOTE(CS, 4), NOTE(DN, 6), NOTE(DS, 6), NOTE(EN, 6),
  NOTE(DN, 4), NOTE(FN, 4), NOTE(GS, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(FN, 5), NOTE(GS, 5), NOTE(BN, 5),
  NOTE(EN, 4), NOTE(GN, 4), NOTE(AS, 4), NOTE(CS, 5), NOTE(EN, 5), NOTE(GN, 5), NOTE(AS, 5), NOTE(CS, 6),
  NOTE(FS, 4), NOTE(AN, 4), NOTE(CN, 5), NOTE(DS, 5), NOTE(FS, 5), NOTE(AN, 5), NOTE(CN, 6), NOTE(DS, 6)
};

const uint8_t* manoury1LayoutLeftClose = manoury1LayoutLeftOpen;
const uint8_t* manoury1LayoutRightClose = manoury1LayoutRightOpen;

NoteLayout manoury1NoteLayout = { manoury1LayoutLeftOpen, manoury1LayoutRightOpen, manoury1LayoutLeftClose, manoury1LayoutRightClose, gNoteLayoutNames[NOTELAYOUTTYPE_MANOURY1] };

//====================================================================================================
// Manoury 2 layout
//====================================================================================================
const uint8_t manoury2LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(GS, 6), NOTE(AN, 6), NOTE(AS, 6), NOTE(BN, 6),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(BN, 3), NOTE(CN, 4), NOTE(FN, 6), NOTE(FS, 6), NOTE(GN, 6),
  NOTE_UNUSED, NOTE(AS, 3), NOTE(CS, 4), NOTE(DN, 4), NOTE(DS, 4), NOTE(DN, 6), NOTE(DS, 6), NOTE(EN, 6),
  NOTE(AN, 3), NOTE(FN, 4), NOTE(GS, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(FN, 5), NOTE(GS, 5), NOTE(BN, 5),
  NOTE(EN, 4), NOTE(GN, 4), NOTE(AS, 4), NOTE(CS, 5), NOTE(EN, 5), NOTE(GN, 5), NOTE(AS, 5), NOTE(CS, 6),
  NOTE(FS, 4), NOTE(AN, 4), NOTE(CN, 5), NOTE(DS, 5), NOTE(FS, 5), NOTE(AN, 5), NOTE(CN, 6), NOTE(DN, 6)
};
const uint8_t* manoury2LayoutLeftOpen = manoury1LayoutLeftOpen;
const uint8_t* manoury2LayoutLeftClose = manoury2LayoutLeftOpen;
const uint8_t* manoury2LayoutRightClose = manoury2LayoutRightOpen;

NoteLayout manoury2NoteLayout = { manoury2LayoutLeftOpen, manoury2LayoutRightOpen, manoury2LayoutLeftClose, manoury2LayoutRightClose, gNoteLayoutNames[NOTELAYOUTTYPE_MANOURY2] };

//====================================================================================================
// Tango 142
//====================================================================================================
const uint8_t tango142LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(GS, 2), NOTE(AS, 2), NOTE(CS, 3), NOTE(FN, 3), NOTE(GS, 4),
  NOTE_UNUSED, NOTE(EN, 2), NOTE(AN, 2), NOTE(GN, 3), NOTE(DS, 3), NOTE(FN, 4), NOTE(AS, 3), NOTE(FN, 2),
  NOTE(DN, 2), NOTE(DN, 3), NOTE(AN, 3), NOTE(CN, 4), NOTE(EN, 4), NOTE(CN, 3), NOTE(GN, 2), NOTE_UNUSED,
  NOTE(EN, 3), NOTE(GS, 3), NOTE(BN, 3), NOTE(DN, 4), NOTE(FS, 4), NOTE(CS, 4), NOTE(FS, 2), NOTE_UNUSED,
  NOTE(BN, 2), NOTE(GN, 4), NOTE(AN, 4), NOTE(DS, 4), NOTE(FS, 3), NOTE(DS, 2), NOTE(CN, 2), NOTE_UNUSED
};

const uint8_t tango142LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(BN, 6), NOTE(GS, 6), NOTE(GN, 6), NOTE(FN, 6),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(CS, 4), NOTE(AN, 6), NOTE(FS, 6), NOTE(EN, 6), NOTE(DS, 6),
  NOTE_UNUSED, NOTE_UNUSED, NOTE(CN, 4), NOTE(DN, 4), NOTE(GN, 4), NOTE(AS, 5), NOTE(CN, 6), NOTE(DN, 6),
  NOTE_UNUSED, NOTE(BN, 3), NOTE(EN, 4), NOTE(CS, 5), NOTE(FS, 4), NOTE(AN, 4), NOTE(CN, 5), NOTE(EN, 5),
  NOTE(AN, 3), NOTE(FN, 4), NOTE(AS, 4), NOTE(GS, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(GS, 5), NOTE(BN, 5),
  NOTE(AS, 3), NOTE(DS, 4), NOTE(FN, 5), NOTE(DS, 5), NOTE(FS, 5), NOTE(AN, 5), NOTE(CS, 6), NOTE(GN, 5)
};

const uint8_t tango142LayoutLeftClose[PinInputs::keyCounts[LEFT]] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(GS, 2), NOTE(AS, 2), NOTE(DS, 3), NOTE(DS, 4), NOTE(GN, 4),
  NOTE_UNUSED, NOTE(DN, 2), NOTE(DN, 3), NOTE(AS, 3), NOTE(CN, 4), NOTE(CS, 3), NOTE(CN, 3), NOTE(FS, 2),
  NOTE(EN, 2), NOTE(GN, 2), NOTE(GN, 3), NOTE(BN, 3), NOTE(DN, 4), NOTE(FN, 4), NOTE(FS, 3), NOTE_UNUSED,
  NOTE(AN, 2), NOTE(EN, 3), NOTE(AN, 3), NOTE(CS, 4), NOTE(EN, 4), NOTE(GS, 3), NOTE(BN, 2), NOTE_UNUSED,
  NOTE(EN, 3), NOTE(FS, 4), NOTE(GS, 4), NOTE(BN, 4), NOTE(FN, 3), NOTE(CS, 2), NOTE(FN, 2), NOTE_UNUSED
};

const uint8_t tango142LayoutRightClose[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(AN, 6), NOTE(GS, 6), NOTE(FS, 6), NOTE(FN, 6),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(CN, 4), NOTE(GN, 6), NOTE(AS, 5), NOTE(CN, 6), NOTE(DS, 6),
  NOTE_UNUSED, NOTE_UNUSED, NOTE(DN, 4), NOTE(CS, 4), NOTE(GS, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(DN, 6),
  NOTE_UNUSED, NOTE(BN, 3), NOTE(FS, 4), NOTE(FS, 5), NOTE(GN, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(GN, 5),
  NOTE(AN, 3), NOTE(FN, 4), NOTE(EN, 4), NOTE(AN, 4), NOTE(CS, 5), NOTE(EN, 5), NOTE(AN, 5), NOTE(CS, 6),
  NOTE(AS, 3), NOTE(DS, 4), NOTE(FN, 5), NOTE(EN, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(EN, 6), NOTE(DS, 5)
};

NoteLayout tango142NoteLayout = { tango142LayoutLeftOpen, tango142LayoutRightOpen, tango142LayoutLeftClose, tango142LayoutRightClose, gNoteLayoutNames[NOTELAYOUTTYPE_TANGO_142] };

//====================================================================================================
// Hayden layout without the lowest notes
//====================================================================================================
const uint8_t hayden1LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE(DS, 4), NOTE(FN, 4), NOTE(GN, 4), NOTE(AN, 4), NOTE(BN, 4), NOTE(CS, 5),
  SKIPPED_KEY, NOTE(GS, 3), NOTE(AS, 3), NOTE(CN, 4), NOTE(DN, 4), NOTE(EN, 4), NOTE(FS, 4), NOTE(GS, 4),
  NOTE(CS, 2), NOTE(DS, 3), NOTE(FN, 3), NOTE(GN, 3), NOTE(AN, 3), NOTE(BN, 3), NOTE(CS, 4), NOTE(DS, 4),
  NOTE(GS, 2), NOTE(AS, 2), NOTE(CN, 3), NOTE(DN, 3), NOTE(EN, 3), NOTE(FS, 3), NOTE(GS, 3), NOTE(AS, 3),
  NOTE(DS, 2), NOTE(FN, 2), NOTE(GN, 2), NOTE(AN, 2), NOTE(BN, 2), NOTE(CS, 3), NOTE(DS, 3), NOTE(FN, 3)
};

const uint8_t hayden1LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE(DS, 6), NOTE(FN, 6), NOTE(GN, 6), NOTE(AN, 6), NOTE(BN, 6),
  SKIPPED_KEY, NOTE(FS, 5), NOTE(GS, 5), NOTE(AS, 5), NOTE(CN, 6), NOTE(DN, 6), NOTE(EN, 6), NOTE(FS, 6),  // Use the potential button as note FS6
  NOTE(FS, 3), NOTE(CS, 5), NOTE(DS, 5), NOTE(FN, 5), NOTE(GN, 5), NOTE(AN, 5), NOTE(BN, 5), NOTE(CS, 6),
  NOTE(FS, 4), NOTE(GS, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(DN, 5), NOTE(EN, 5), NOTE(FS, 5), NOTE(GS, 5),
  NOTE(CS, 4), NOTE(DS, 4), NOTE(FN, 4), NOTE(GN, 4), NOTE(AN, 4), NOTE(BN, 4), NOTE(CS, 5), NOTE(DS, 5),
  NOTE(GS, 3), NOTE(AS, 3), NOTE(CN, 4), NOTE(DN, 4), NOTE(EN, 4), NOTE(FS, 4), NOTE(GS, 4), NOTE(AS, 4)
};
const uint8_t* hayden1LayoutLeftClose = hayden1LayoutLeftOpen;
const uint8_t* hayden1LayoutRightClose = hayden1LayoutRightOpen;

NoteLayout hayden1NoteLayout = { hayden1LayoutLeftOpen, hayden1LayoutRightOpen, hayden1LayoutLeftClose, hayden1LayoutRightClose, gNoteLayoutNames[NOTELAYOUTTYPE_HAYDEN1] };

//====================================================================================================
// Hayden layout with low notes on both sides
//====================================================================================================
const uint8_t hayden2LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE(AS, 3), NOTE(CN, 4), NOTE(DN, 4), NOTE(EN, 4), NOTE(FS, 4), NOTE(GS, 4),
  SKIPPED_KEY, NOTE(DS, 3), NOTE(FN, 3), NOTE(GN, 3), NOTE(AN, 3), NOTE(BN, 3), NOTE(CS, 4), NOTE(DS, 4),
  NOTE(GS, 1), NOTE(AS, 2), NOTE(CN, 3), NOTE(DN, 3), NOTE(EN, 3), NOTE(FS, 3), NOTE(GS, 3), NOTE(AS, 3),
  NOTE(DS, 2), NOTE(FN, 2), NOTE(GN, 2), NOTE(AN, 2), NOTE(BN, 2), NOTE(CS, 3), NOTE(DS, 3), NOTE(FN, 3),
  NOTE(AS, 1), NOTE(CN, 2), NOTE(DN, 2), NOTE(EN, 2), NOTE(FS, 2), NOTE(GS, 2), NOTE(AS, 2), NOTE(CN, 3)
};

const uint8_t hayden2LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE(GS, 5), NOTE(AS, 5), NOTE(CN, 6), NOTE(DN, 6), NOTE(EN, 6), NOTE(FS, 6),
  NOTE(CS, 2), NOTE(CS, 5), NOTE(DS, 5), NOTE(FN, 5), NOTE(GN, 5), NOTE(AN, 5), NOTE(BN, 5), NOTE(CS, 6),
  NOTE(CS, 3), NOTE(GS, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(DN, 5), NOTE(EN, 5), NOTE(FS, 5), NOTE(GS, 5),
  NOTE(CS, 4), NOTE(DS, 4), NOTE(FN, 4), NOTE(GN, 4), NOTE(AN, 4), NOTE(BN, 4), NOTE(CS, 5), NOTE(DS, 5),
  NOTE(GS, 3), NOTE(AS, 3), NOTE(CN, 4), NOTE(DN, 4), NOTE(EN, 4), NOTE(FS, 4), NOTE(GS, 4), NOTE(AS, 4),
  NOTE(DS, 3), NOTE(FN, 3), NOTE(GN, 3), NOTE(AN, 3), NOTE(BN, 3), NOTE(CS, 4), NOTE(DS, 4), NOTE(FN, 4)
};

const uint8_t* hayden2LayoutLeftClose = hayden2LayoutLeftOpen;
const uint8_t* hayden2LayoutRightClose = hayden2LayoutRightOpen;

NoteLayout hayden2NoteLayout = { hayden2LayoutLeftOpen, hayden2LayoutRightOpen, hayden2LayoutLeftClose, hayden2LayoutRightClose, gNoteLayoutNames[NOTELAYOUTTYPE_HAYDEN2] };

//====================================================================================================
void syncNoteLayout() {
  if (gBigState.mNoteLayout.mName != getNoteLayoutName()) {
    Serial.printf("Switching to %s\n", getNoteLayoutName());
    switch (gSettings.noteLayout) {
      case NOTELAYOUTTYPE_MANOURY1:
        gBigState.mNoteLayout = manoury1NoteLayout;
        break;
      case NOTELAYOUTTYPE_MANOURY2:
        gBigState.mNoteLayout = manoury2NoteLayout;
        break;
      case NOTELAYOUTTYPE_TANGO_142:
        gBigState.mNoteLayout = tango142NoteLayout;
        break;
      case NOTELAYOUTTYPE_HAYDEN1:
        gBigState.mNoteLayout = hayden1NoteLayout;
        break;
      case NOTELAYOUTTYPE_HAYDEN2:
        gBigState.mNoteLayout = hayden2NoteLayout;
        break;
      default:
        Serial.printf("Unknown note layout %d\n", gSettings.noteLayout);
        break;
    }
    gSettings.updateMIDIRange();
  }
}
