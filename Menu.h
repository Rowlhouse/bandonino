#ifndef MENU_H
#define MENU_H

class Adafruit_SSD1327;
extern Adafruit_SSD1327 display;

struct State;
struct Settings;

// Call this once
void initMenu();

// Call this every tick
void updateMenu();

// clear screen and show message for time (in ms). This will block.
void showMessage(const char* msg, int time);

#endif