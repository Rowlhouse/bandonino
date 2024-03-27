#ifndef PININPUTS_H
#define PININPUTS_H

#include <wiring.h>

struct PinInputs {
  static const int rowCountLeft = 8;
  static const int rowCountRight = 8;
  static const int columnCountLeft = 5;
  static const int columnCountRight = 6;

  static constexpr byte columnPinsLeft[columnCountLeft] = { 0, 2, 3, 4, 5 };
  static constexpr byte columnPinsRight[columnCountRight] = { 6, 7, 8, 9, 10, 11 };
  static constexpr byte rowPinsLeft[rowCountLeft] = { 32, 31, 30, 29, 28, 27, 26, 25 };
  static constexpr byte rowPinsRight[rowCountRight] = { 33, 34, 35, 36, 37, 38, 39, 40 };

  static const int keyCountLeft = rowCountLeft * columnCountLeft;
  static const int keyCountRight = rowCountRight * columnCountRight;
};

inline int toKeyIndex(int row, int column, int rowCount, int columnCount) {
  return (column * rowCount) + row;
}
#define INDEX_LEFT(row, column) toKeyIndex(row, column, PinInputs::rowCountLeft, PinInputs::columnCountLeft)
#define INDEX_RIGHT(row, column) toKeyIndex(row, column, PinInputs::rowCountRight, PinInputs::columnCountRight)

const int LOADCELL_DOUT_PIN = 15;
const int LOADCELL_SCK_PIN = 16;

#endif
