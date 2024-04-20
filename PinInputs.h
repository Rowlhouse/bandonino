#ifndef PININPUTS_H
#define PININPUTS_H

#include <stdint.h>

namespace PinInputs {
static constexpr int rowCounts[2] = { 8, 8 };
static constexpr int columnCounts[2] = { 5, 6 };

static constexpr uint8_t columnPinsLeft[columnCounts[0]] = { 5, 4, 3, 2, 0 };
static constexpr uint8_t columnPinsRight[columnCounts[1]] = { 11, 10, 9, 8, 7, 6 };
static constexpr uint8_t rowPinsLeft[rowCounts[0]] = { 32, 31, 30, 29, 28, 27, 26, 25 };
static constexpr uint8_t rowPinsRight[rowCounts[1]] = { 40, 39, 38, 37, 36, 35, 34, 33 };

inline const uint8_t* columnPins(int side) {
  return side ? columnPinsRight : columnPinsLeft;
}
inline const uint8_t* rowPins(int side) {
  return side ? rowPinsRight : rowPinsLeft;
}

static constexpr int keyCounts[2] = { rowCounts[0] * columnCounts[0], rowCounts[1] * columnCounts[1] };
};

inline int toKeyIndex(int row, int column, int rowCount, int columnCount) {
  return (column * rowCount) + row;
}
#define INDEX_LEFT(row, column) toKeyIndex(row, column, PinInputs::rowCounts[0], PinInputs::columnCounts[0])
#define INDEX_RIGHT(row, column) toKeyIndex(row, column, PinInputs::rowCounts[1], PinInputs::columnCounts[1])

#define LOADCELL_DOUT_PIN 15
#define LOADCELL_SCK_PIN 16

// Digital input pins
#define ROTARY_ENCODER_BUTTON_PIN 17
#define ROTARY_PIN1 A9
#define ROTARY_PIN2 A8


#endif
