#ifndef NOTELAYOUTS_H
#define NOTELAYOUTS_H

#include <wiring.h>

struct Settings;

enum NoteLayoutType {
  NOTELAYOUTTYPE_MANOURY,
  NOTELAYOUTTYPE_MANOURY2,
  NOTELAYOUTTYPE_TANGO_142,
  NOTELAYOUTTYPE_NUM,
};

enum { LEFT,
       RIGHT };

struct NoteLayout {
  const byte* leftOpen = nullptr;
  const byte* rightOpen = nullptr;
  const byte* leftClose = nullptr;
  const byte* rightClose = nullptr;
  const byte* open(int side) {
    return side ? rightOpen : leftOpen;
  }
  const byte* close(int side) {
    return side ? rightClose : leftClose;
  }
  const char* name = "none";
};

extern const char* gNoteLayouts[];

const char* getNoteLayout();

// This updates the state to have the note arrays to match what is in settings
void syncNoteLayout();

#endif
