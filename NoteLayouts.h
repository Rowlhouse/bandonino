#ifndef NOTELAYOUTS_H
#define NOTELAYOUTS_H

struct Settings;

enum NoteLayoutType {
  NOTELAYOUTTYPE_MANOURY,
  NOTELAYOUTTYPE_NUM,
  NOTELAYOUTTYPE_MAX = NOTELAYOUTTYPE_NUM - 1
};

extern const char* gNoteLayouts[];

// This updates the state to have the note arrays to match what is in settings
void SyncNoteLayout();

#endif
