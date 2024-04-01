#include "NoteLayouts.h"

#include "NoteNames.h"
#include "PinInputs.h"
#include "Settings.h"
#include "State.h"

#include "usb_serial.h"

const char* gNoteLayouts[] = {
  "Manoury",
  "Tango142"
};

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
const byte manouryLayoutLeftOpen[PinInputs::keyCountLeft] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE(DN, 3), NOTE(CS, 3), NOTE(CN, 3), NOTE(GS, 3), NOTE(GN, 3), NOTE(FS, 3),
  NOTE_UNUSED, NOTE(GS, 5), NOTE(FN, 3), NOTE(EN, 3), NOTE(DS, 3), NOTE(BN, 3), NOTE(AS, 3), NOTE(AN, 3),
  NOTE(AN, 5), NOTE(FN, 5), NOTE(DN, 5), NOTE(BN, 4), NOTE(GS, 4), NOTE(FN, 4), NOTE(DN, 4), NOTE_UNUSED,
  NOTE(GN, 5), NOTE(EN, 5), NOTE(CS, 5), NOTE(AS, 4), NOTE(GN, 4), NOTE(EN, 4), NOTE(CS, 4), NOTE_UNUSED,
  NOTE(FS, 5), NOTE(DS, 5), NOTE(CN, 5), NOTE(AN, 4), NOTE(FS, 4), NOTE(DS, 4), NOTE(CN, 4), NOTE_UNUSED
};

const byte manouryLayoutRightOpen[PinInputs::keyCountRight] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(GS, 7), NOTE(AN, 7), NOTE(AS, 7), NOTE(BN, 7),
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(AN, 4), NOTE(AS, 4), NOTE(FN, 7), NOTE(FS, 7), NOTE(GN, 7),
  NOTE(DS, 5), NOTE_UNUSED, NOTE(BN, 4), NOTE(CN, 5), NOTE(CS, 5), NOTE(DN, 7), NOTE(DS, 7), NOTE(EN, 7),
  NOTE(DN, 5), NOTE(FN, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(DN, 6), NOTE(FN, 6), NOTE(GS, 6), NOTE(BN, 6),
  NOTE(EN, 5), NOTE(GN, 5), NOTE(AS, 5), NOTE(CS, 6), NOTE(EN, 6), NOTE(GN, 6), NOTE(AS, 6), NOTE(CS, 7),
  NOTE(FS, 5), NOTE(AN, 5), NOTE(CN, 6), NOTE(DS, 6), NOTE(FS, 6), NOTE(AN, 6), NOTE(CN, 7), NOTE(DS, 7)
};
const byte* manouryLayoutLeftClose = manouryLayoutLeftOpen;
const byte* manouryLayoutRightClose = manouryLayoutRightOpen;

//====================================================================================================
// Tango 142
//====================================================================================================
const byte tango142LayoutLeftOpen[PinInputs::keyCountLeft] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(GS, 3), NOTE(AS, 3), NOTE(CS, 4), NOTE(FN, 4), NOTE(GS, 5),
  NOTE_UNUSED, NOTE(EN, 3), NOTE(AN, 3), NOTE(GN, 4), NOTE(DS, 4), NOTE(FN, 5), NOTE(AS, 4), NOTE(FN, 3),
  NOTE(DN, 3), NOTE(DN, 4), NOTE(AN, 4), NOTE(CN, 5), NOTE(EN, 5), NOTE(CN, 4), NOTE(GN, 3), NOTE_UNUSED,
  NOTE(EN, 4), NOTE(GS, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(FS, 5), NOTE(CS, 5), NOTE(FS, 3), NOTE_UNUSED,
  NOTE(BN, 3), NOTE(GN, 5), NOTE(AN, 5), NOTE(DS, 5), NOTE(FS, 4), NOTE(DS, 3), NOTE(CN, 3), NOTE_UNUSED
};

const byte tango142LayoutRightOpen[PinInputs::keyCountRight] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(BN, 7), NOTE(GS, 7), NOTE(GN, 7), NOTE(FN, 7),
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(CS, 5), NOTE(AN, 7), NOTE(FS, 7), NOTE(EN, 7), NOTE(DS, 7),
  NOTE_UNUSED, NOTE_UNUSED, NOTE(CN, 5), NOTE(DN, 5), NOTE(GN, 5), NOTE(AS, 6), NOTE(CN, 7), NOTE(DN, 7),
  NOTE_UNUSED, NOTE(BN, 4), NOTE(EN, 5), NOTE(CS, 6), NOTE(FS, 5), NOTE(AN, 5), NOTE(CN, 6), NOTE(EN, 6),
  NOTE(AN, 4), NOTE(FN, 5), NOTE(AS, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(DN, 6), NOTE(GS, 6), NOTE(BN, 6),
  NOTE(AS, 4), NOTE(DS, 5), NOTE(FN, 6), NOTE(DS, 6), NOTE(FS, 6), NOTE(AN, 6), NOTE(CS, 7), NOTE(GN, 6)
};

const byte tango142LayoutLeftClose[PinInputs::keyCountLeft] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(GS, 3), NOTE(AS, 3), NOTE(DS, 4), NOTE(DS, 5), NOTE(GN, 5),
  NOTE_UNUSED, NOTE(DN, 3), NOTE(DN, 4), NOTE(AS, 4), NOTE(CN, 5), NOTE(CS, 4), NOTE(CN, 4), NOTE(FS, 3),
  NOTE(EN, 3), NOTE(GN, 3), NOTE(GN, 4), NOTE(BN, 4), NOTE(DN, 5), NOTE(FN, 5), NOTE(FS, 4), NOTE_UNUSED,
  NOTE(AN, 3), NOTE(EN, 4), NOTE(AN, 4), NOTE(CS, 5), NOTE(EN, 5), NOTE(GS, 4), NOTE(BN, 3), NOTE_UNUSED,
  NOTE(EN, 4), NOTE(FS, 5), NOTE(GS, 5), NOTE(BN, 5), NOTE(FN, 4), NOTE(CS, 3), NOTE(FN, 3), NOTE_UNUSED
};

const byte tango142LayoutRightClose[PinInputs::keyCountRight] = {
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(AN, 7), NOTE(GS, 7), NOTE(FS, 7), NOTE(FN, 7),
  NOTE_UNUSED, NOTE_UNUSED, NOTE_UNUSED, NOTE(CN, 5), NOTE(GN, 7), NOTE(AS, 6), NOTE(CN, 7), NOTE(DS, 7),
  NOTE_UNUSED, NOTE_UNUSED, NOTE(DN, 5), NOTE(CS, 5), NOTE(GS, 5), NOTE(AS, 5), NOTE(CN, 6), NOTE(DN, 7),
  NOTE_UNUSED, NOTE(BN, 4), NOTE(FS, 5), NOTE(FS, 6), NOTE(GN, 5), NOTE(BN, 5), NOTE(DN, 6), NOTE(GN, 6),
  NOTE(AN, 4), NOTE(FN, 5), NOTE(EN, 5), NOTE(AN, 5), NOTE(CS, 6), NOTE(EN, 6), NOTE(AN, 6), NOTE(CS, 7),
  NOTE(AS, 4), NOTE(DS, 5), NOTE(FN, 6), NOTE(EN, 6), NOTE(GS, 6), NOTE(BN, 6), NOTE(EN, 7), NOTE(DS, 6)
};

//====================================================================================================
void SyncNoteLayout() {
  switch (settings.noteLayout) {
    case NOTELAYOUTTYPE_MANOURY:
      if (bigState.noteLayoutLeftClose != manouryLayoutLeftClose) {
        Serial.println("Switching to Manoury");
        bigState.noteLayoutLeftClose = manouryLayoutLeftClose;
        bigState.noteLayoutLeftOpen = manouryLayoutLeftOpen;
        bigState.noteLayoutRightClose = manouryLayoutRightClose;
        bigState.noteLayoutRightOpen = manouryLayoutRightOpen;
        settings.updateMIDIRange();
      }
      break;
    case NOTELAYOUTTYPE_TANGO_142:
      if (bigState.noteLayoutLeftClose != tango142LayoutLeftClose) {
        Serial.println("Switching to Tango142");
        bigState.noteLayoutLeftClose = tango142LayoutLeftClose;
        bigState.noteLayoutLeftOpen = tango142LayoutLeftOpen;
        bigState.noteLayoutRightClose = tango142LayoutRightClose;
        bigState.noteLayoutRightOpen = tango142LayoutRightOpen;
        settings.updateMIDIRange();
      }
      break;
    default:
      break;
  }
}
