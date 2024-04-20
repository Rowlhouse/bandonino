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
  const uint8_t* mLeftOpen = nullptr;
  const uint8_t* mRightOpen = nullptr;
  const uint8_t* mLeftClose = nullptr;
  const uint8_t* mRightClose = nullptr;
  const uint8_t* open(int side) {
    return side ? mRightOpen : mLeftOpen;
  }
  const uint8_t* close(int side) {
    return side ? mRightClose : mLeftClose;
  }
  const char* mName = "none";
};

extern const char* gNoteLayoutNames[];

const char* getNoteLayoutName();

// This updates the state to have the note arrays to match what is in settings
void syncNoteLayout();

// The "action" keys - i.e. hot keys that could be mapped to things like zeroBellows. 
// Check them with something like gBigState.activeKeysRight[gActionKey1]
extern const int gActionKey1;
extern const int gActionKey2;



#endif
