#ifndef PININPUTS_H
#define PININPUTS_H

#include <wiring.h>

struct PinInputs {
  static constexpr int rowCounts[2] = { 8, 8 };
  static constexpr int columnCounts[2] = { 5, 6 };

  static constexpr byte columnPinsLeft[columnCounts[0]] = { 5, 4, 3, 2, 0 };
  static constexpr byte columnPinsRight[columnCounts[1]] = { 11, 10, 9, 8, 7, 6 };
  static constexpr byte rowPinsLeft[rowCounts[0]] = { 32, 31, 30, 29, 28, 27, 26, 25 };
  static constexpr byte rowPinsRight[rowCounts[1]] = { 40, 39, 38, 37, 36, 35, 34, 33 };

  static const byte* columnPins(int side) {
    return side ? columnPinsRight : columnPinsLeft;
  }
  static const byte* rowPins(int side) {
    return side ? rowPinsRight : rowPinsLeft;
  }

  static constexpr int keyCounts[2] = { rowCounts[0] * columnCounts[0], rowCounts[1] * columnCounts[1] };
};

inline int toKeyIndex(int row, int column, int rowCount, int columnCount) {
  return (column * rowCount) + row;
}
#define INDEX_LEFT(row, column) toKeyIndex(row, column, PinInputs::rowCounts[0], PinInputs::columnCounts[0])
#define INDEX_RIGHT(row, column) toKeyIndex(row, column, PinInputs::rowCounts[1], PinInputs::columnCounts[1])

const int LOADCELL_DOUT_PIN = 15;
const int LOADCELL_SCK_PIN = 16;

#endif
