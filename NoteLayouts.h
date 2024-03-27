#ifndef NOTELAYOUTS_H
#define NOTELAYOUTS_H

struct Settings;

enum NoteLayoutType {
  LAYOUT_MANOURY,
  LAYOUT_MAX
};

void SetNoteLayout(NoteLayoutType type, Settings& settings);

#endif
