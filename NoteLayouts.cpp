#include "NoteLayouts.h"

#include "NoteNames.h"
#include "PinInputs.h"
#include "Settings.h"
#include "State.h"

#include "usb_serial.h"

const char* gNoteLayouts[] = {
  "Manoury1",
  "Manoury2",
  "Tango142",
  "Hayden1",
  "Hayden2"
};

//====================================================================================================
const char* getNoteLayout() {
  return gNoteLayouts[settings.noteLayout];
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
const byte manoury1LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE(DN, 3), NOTE(CS, 3), NOTE(CN, 3), NOTE(GS, 3), NOTE(GN, 3), NOTE(FS, 3),
  NOTE_UNUSED, NOTE(GS, 5), NOTE(FN, 3), NOTE(EN, 3), NOTE(DS, 3), NOTE(BN, 3), NOTE(AS, 3), NOTE(AN, 3),
  NOTE(AN, 5), NOTE(FN, 5), NOTE(DN, 5), NOTE(BN, 4), NOTE(GS, 4), NOTE(FN, 4), NOTE(DN, 4), NOTE_UNUSED,
  NOTE(GN, 5), NOTE(EN, 5), NOTE(CS, 5), NOTE(AS, 4), NOTE(GN, 4), NOTE(EN, 4), NOTE(CS, 4), NOTE_UNUSED,
  NOTE(FS, 5), NOTE(DS, 5), NOTE(CN, 5), NOTE(AN, 4), NOTE(FS, 4), NOTE(DS, 4), NOTE(CN, 4), NOTE_UNUSED
};

const byte manoury1LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(GS, 7), NOTE(AN, 7), NOTE(AS, 7), NOTE(BN, 7),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(AN, 4), NOTE(AS, 4), NOTE(FN, 7), NOTE(FS, 7), NOTE(GN, 7),
  NOTE(DS, 5), NOTE_UNUSED, NOTE(BN, 4), NOTE(CN, 5), NOTE(CS, 5), NOTE(DN, 7), NOTE(DS, 7), NOTE(EN, 7),
  NOTE(DN, 5), NOTE(FN, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(DN, 6), NOTE(FN, 6), NOTE(GS, 6), NOTE(BN, 6),
  NOTE(EN, 5), NOTE(GN, 5), NOTE(AS, 5), NOTE(CS, 6), NOTE(EN, 6), NOTE(GN, 6), NOTE(AS, 6), NOTE(CS, 7),
  NOTE(FS, 5), NOTE(AN, 5), NOTE(CN, 6), NOTE(DS, 6), NOTE(FS, 6), NOTE(AN, 6), NOTE(CN, 7), NOTE(DS, 7)
};

const byte* manoury1LayoutLeftClose = manoury1LayoutLeftOpen;
const byte* manoury1LayoutRightClose = manoury1LayoutRightOpen;

NoteLayout manoury1NoteLayout = { manoury1LayoutLeftOpen, manoury1LayoutRightOpen, manoury1LayoutLeftClose, manoury1LayoutRightClose, gNoteLayouts[NOTELAYOUTTYPE_MANOURY1] };

//====================================================================================================
// Manoury 2 layout
//====================================================================================================
const byte manoury2LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(GS, 7), NOTE(AN, 7), NOTE(AS, 7), NOTE(BN, 7),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(BN, 4), NOTE(CN, 5), NOTE(FN, 7), NOTE(FS, 7), NOTE(GN, 7),
  NOTE_UNUSED, NOTE(AS, 4), NOTE(CS, 5), NOTE(DN, 5), NOTE(DS, 5), NOTE(DN, 7), NOTE(DS, 7), NOTE(EN, 7),
  NOTE(AN, 4), NOTE(FN, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(DN, 6), NOTE(FN, 6), NOTE(GS, 6), NOTE(BN, 6),
  NOTE(EN, 5), NOTE(GN, 5), NOTE(AS, 5), NOTE(CS, 6), NOTE(EN, 6), NOTE(GN, 6), NOTE(AS, 6), NOTE(CS, 7),
  NOTE(FS, 5), NOTE(AN, 5), NOTE(CN, 6), NOTE(DS, 6), NOTE(FS, 6), NOTE(AN, 6), NOTE(CN, 7), NOTE(DN, 7)
};
const byte* manoury2LayoutLeftOpen = manoury1LayoutLeftOpen;
const byte* manoury2LayoutLeftClose = manoury2LayoutLeftOpen;
const byte* manoury2LayoutRightClose = manoury2LayoutRightOpen;

NoteLayout manoury2NoteLayout = { manoury2LayoutLeftOpen, manoury2LayoutRightOpen, manoury2LayoutLeftClose, manoury2LayoutRightClose, gNoteLayouts[NOTELAYOUTTYPE_MANOURY2] };

//====================================================================================================
// Tango 142
//====================================================================================================
const byte tango142LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(GS, 3), NOTE(AS, 3), NOTE(CS, 4), NOTE(FN, 4), NOTE(GS, 5),
  NOTE_UNUSED, NOTE(EN, 3), NOTE(AN, 3), NOTE(GN, 4), NOTE(DS, 4), NOTE(FN, 5), NOTE(AS, 4), NOTE(FN, 3),
  NOTE(DN, 3), NOTE(DN, 4), NOTE(AN, 4), NOTE(CN, 5), NOTE(EN, 5), NOTE(CN, 4), NOTE(GN, 3), NOTE_UNUSED,
  NOTE(EN, 4), NOTE(GS, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(FS, 5), NOTE(CS, 5), NOTE(FS, 3), NOTE_UNUSED,
  NOTE(BN, 3), NOTE(GN, 5), NOTE(AN, 5), NOTE(DS, 5), NOTE(FS, 4), NOTE(DS, 3), NOTE(CN, 3), NOTE_UNUSED
};

const byte tango142LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(BN, 7), NOTE(GS, 7), NOTE(GN, 7), NOTE(FN, 7),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(CS, 5), NOTE(AN, 7), NOTE(FS, 7), NOTE(EN, 7), NOTE(DS, 7),
  NOTE_UNUSED, NOTE_UNUSED, NOTE(CN, 5), NOTE(DN, 5), NOTE(GN, 5), NOTE(AS, 6), NOTE(CN, 7), NOTE(DN, 7),
  NOTE_UNUSED, NOTE(BN, 4), NOTE(EN, 5), NOTE(CS, 6), NOTE(FS, 5), NOTE(AN, 5), NOTE(CN, 6), NOTE(EN, 6),
  NOTE(AN, 4), NOTE(FN, 5), NOTE(AS, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(DN, 6), NOTE(GS, 6), NOTE(BN, 6),
  NOTE(AS, 4), NOTE(DS, 5), NOTE(FN, 6), NOTE(DS, 6), NOTE(FS, 6), NOTE(AN, 6), NOTE(CS, 7), NOTE(GN, 6)
};

const byte tango142LayoutLeftClose[PinInputs::keyCounts[LEFT]] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(GS, 3), NOTE(AS, 3), NOTE(DS, 4), NOTE(DS, 5), NOTE(GN, 5),
  NOTE_UNUSED, NOTE(DN, 3), NOTE(DN, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(CS, 4), NOTE(CN, 4), NOTE(FS, 3),
  NOTE(EN, 3), NOTE(GN, 3), NOTE(GN, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(FN, 5), NOTE(FS, 4), NOTE_UNUSED,
  NOTE(AN, 3), NOTE(EN, 4), NOTE(AN, 4), NOTE(CS, 5), NOTE(EN, 5), NOTE(GS, 4), NOTE(BN, 3), NOTE_UNUSED,
  NOTE(EN, 4), NOTE(FS, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(FN, 4), NOTE(CS, 3), NOTE(FN, 3), NOTE_UNUSED
};

const byte tango142LayoutRightClose[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(AN, 7), NOTE(GS, 7), NOTE(FS, 7), NOTE(FN, 7),
  SKIPPED_KEY, NOTE_BUTTON, NOTE_UNUSED, NOTE(CN, 5), NOTE(GN, 7), NOTE(AS, 6), NOTE(CN, 7), NOTE(DS, 7),
  NOTE_UNUSED, NOTE_UNUSED, NOTE(DN, 5), NOTE(CS, 5), NOTE(GS, 5), NOTE(AS, 5), NOTE(CN, 6), NOTE(DN, 7),
  NOTE_UNUSED, NOTE(BN, 4), NOTE(FS, 5), NOTE(FS, 6), NOTE(GN, 5), NOTE(BN, 5), NOTE(DN, 6), NOTE(GN, 6),
  NOTE(AN, 4), NOTE(FN, 5), NOTE(EN, 5), NOTE(AN, 5), NOTE(CS, 6), NOTE(EN, 6), NOTE(AN, 6), NOTE(CS, 7),
  NOTE(AS, 4), NOTE(DS, 5), NOTE(FN, 6), NOTE(EN, 6), NOTE(GS, 6), NOTE(BN, 6), NOTE(EN, 7), NOTE(DS, 6)
};

NoteLayout tango142NoteLayout = { tango142LayoutLeftOpen, tango142LayoutRightOpen, tango142LayoutLeftClose, tango142LayoutRightClose, gNoteLayouts[NOTELAYOUTTYPE_TANGO_142] };

//====================================================================================================
// Hayden layout without the lowest notes
//====================================================================================================
const byte hayden1LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE(DS, 5), NOTE(FN, 5), NOTE(GN, 5), NOTE(AN, 5), NOTE(BN, 5), NOTE(CS, 6),
  SKIPPED_KEY, NOTE(GS, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(DN, 5), NOTE(EN, 5), NOTE(FS, 5), NOTE(GS, 5),
  NOTE(CS, 3), NOTE(DS, 4), NOTE(FN, 4), NOTE(GN, 4), NOTE(AN, 4), NOTE(BN, 4), NOTE(CS, 5), NOTE(DS, 5),
  NOTE(GS, 3), NOTE(AS, 3), NOTE(CN, 4), NOTE(DN, 4), NOTE(EN, 4), NOTE(FS, 4), NOTE(GS, 4), NOTE(AS, 4),
  NOTE(DS, 3), NOTE(FN, 3), NOTE(GN, 3), NOTE(AN, 3), NOTE(BN, 3), NOTE(CS, 4), NOTE(DS, 4), NOTE(FN, 4)
};

const byte hayden1LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE_BUTTON, NOTE(DS, 7), NOTE(FN, 7), NOTE(GN, 7), NOTE(AN, 7), NOTE(BN, 7),
  SKIPPED_KEY, NOTE(FS, 6), NOTE(GS, 6), NOTE(AS, 6), NOTE(CN, 7), NOTE(DN, 7), NOTE(EN, 7), NOTE(FS, 7),  // Use the potential button as note FS6
  NOTE(FS, 4), NOTE(CS, 6), NOTE(DS, 6), NOTE(FN, 6), NOTE(GN, 6), NOTE(AN, 6), NOTE(BN, 6), NOTE(CS, 7),
  NOTE(FS, 5), NOTE(GS, 5), NOTE(AS, 5), NOTE(CN, 6), NOTE(DN, 6), NOTE(EN, 6), NOTE(FS, 6), NOTE(GS, 6),
  NOTE(CS, 5), NOTE(DS, 5), NOTE(FN, 5), NOTE(GN, 5), NOTE(AN, 5), NOTE(BN, 5), NOTE(CS, 6), NOTE(DS, 6),
  NOTE(GS, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(DN, 5), NOTE(EN, 5), NOTE(FS, 5), NOTE(GS, 5), NOTE(AS, 5)
};
const byte* hayden1LayoutLeftClose = hayden1LayoutLeftOpen;
const byte* hayden1LayoutRightClose = hayden1LayoutRightOpen;

NoteLayout hayden1NoteLayout = { hayden1LayoutLeftOpen, hayden1LayoutRightOpen, hayden1LayoutLeftClose, hayden1LayoutRightClose, gNoteLayouts[NOTELAYOUTTYPE_HAYDEN1] };

//====================================================================================================
// Hayden layout with low notes on both sides
//====================================================================================================
const byte hayden2LayoutLeftOpen[PinInputs::keyCounts[LEFT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE(AS, 4), NOTE(CN, 5), NOTE(DN, 5), NOTE(EN, 5), NOTE(FS, 5), NOTE(GS, 5),
  SKIPPED_KEY, NOTE(DS, 4), NOTE(FN, 4), NOTE(GN, 4), NOTE(AN, 4), NOTE(BN, 4), NOTE(CS, 5), NOTE(DS, 5),
  NOTE(GS, 2), NOTE(AS, 3), NOTE(CN, 4), NOTE(DN, 4), NOTE(EN, 4), NOTE(FS, 4), NOTE(GS, 4), NOTE(AS, 4),
  NOTE(DS, 3), NOTE(FN, 3), NOTE(GN, 3), NOTE(AN, 3), NOTE(BN, 3), NOTE(CS, 4), NOTE(DS, 4), NOTE(FN, 4),
  NOTE(AS, 2), NOTE(CN, 3), NOTE(DN, 3), NOTE(EN, 3), NOTE(FS, 3), NOTE(GS, 3), NOTE(AS, 3), NOTE(CN, 4)
};

const byte hayden2LayoutRightOpen[PinInputs::keyCounts[RIGHT]] = {
  SKIPPED_KEY, SKIPPED_KEY, NOTE(GS, 6), NOTE(AS, 6), NOTE(CN, 7), NOTE(DN, 7), NOTE(EN, 7), NOTE(FS, 7),
  NOTE(CS, 3), NOTE(CS, 6), NOTE(DS, 6), NOTE(FN, 6), NOTE(GN, 6), NOTE(AN, 6), NOTE(BN, 6), NOTE(CS, 7),
  NOTE(CS, 4), NOTE(GS, 5), NOTE(AS, 5), NOTE(CN, 6), NOTE(DN, 6), NOTE(EN, 6), NOTE(FS, 6), NOTE(GS, 6),
  NOTE(CS, 5), NOTE(DS, 5), NOTE(FN, 5), NOTE(GN, 5), NOTE(AN, 5), NOTE(BN, 5), NOTE(CS, 6), NOTE(DS, 6),
  NOTE(GS, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(DN, 5), NOTE(EN, 5), NOTE(FS, 5), NOTE(GS, 5), NOTE(AS, 5),
  NOTE(DS, 4), NOTE(FN, 4), NOTE(GN, 4), NOTE(AN, 4), NOTE(BN, 4), NOTE(CS, 5), NOTE(DS, 5), NOTE(FN, 5)
};

const byte* hayden2LayoutLeftClose = hayden2LayoutLeftOpen;
const byte* hayden2LayoutRightClose = hayden2LayoutRightOpen;

NoteLayout hayden2NoteLayout = { hayden2LayoutLeftOpen, hayden2LayoutRightOpen, hayden2LayoutLeftClose, hayden2LayoutRightClose, gNoteLayouts[NOTELAYOUTTYPE_HAYDEN2] };

//====================================================================================================
void syncNoteLayout() {
  if (bigState.noteLayout.name != getNoteLayout()) {
    Serial.printf("Switching to %s\n", getNoteLayout());
    switch (settings.noteLayout) {
      case NOTELAYOUTTYPE_MANOURY1:
        bigState.noteLayout = manoury1NoteLayout;
        break;
      case NOTELAYOUTTYPE_MANOURY2:
        bigState.noteLayout = manoury2NoteLayout;
        break;
      case NOTELAYOUTTYPE_TANGO_142:
        bigState.noteLayout = tango142NoteLayout;
        break;
      case NOTELAYOUTTYPE_HAYDEN1:
        bigState.noteLayout = hayden1NoteLayout;
        break;
      case NOTELAYOUTTYPE_HAYDEN2:
        bigState.noteLayout = hayden2NoteLayout;
        break;
      default:
        Serial.printf("Unknown note layout %d\n", settings.noteLayout);
        break;
    }
    settings.updateMIDIRange();
  }
}
