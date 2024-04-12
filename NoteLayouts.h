#ifndef NOTELAYOUTS_H
#define NOTELAYOUTS_H

#include <stdint.h>

struct Settings;

enum NoteLayoutType {
  NOTELAYOUTTYPE_MANOURY1,
  NOTELAYOUTTYPE_MANOURY2,
  NOTELAYOUTTYPE_TANGO_142,
  NOTELAYOUTTYPE_HAYDEN1,
  NOTELAYOUTTYPE_HAYDEN2,
  NOTELAYOUTTYPE_NUM,
};

enum { LEFT,
       RIGHT };

struct NoteLayout {
  const uint8_t* leftOpen = nullptr;
  const uint8_t* rightOpen = nullptr;
  const uint8_t* leftClose = nullptr;
  const uint8_t* rightClose = nullptr;
  const uint8_t* open(int side) {
    return side ? rightOpen : leftOpen;
  }
  const uint8_t* close(int side) {
    return side ? rightClose : leftClose;
  }
  const char* name = "none";
};

extern const char* gNoteLayouts[];

const char* getNoteLayout();

// This updates the state to have the note arrays to match what is in settings
void syncNoteLayout();

#endif
