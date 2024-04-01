#ifndef MENU_H
#define MENU_H

// https://github.com/adafruit/Adafruit_SSD1327
// v 1.0.4
#include <Adafruit_SSD1327.h>
extern Adafruit_SSD1327 display;

struct State;
struct Settings;

void initMenu();

void updateMenu(Settings& settings, State& state);

void forceMenuRefresh();

#endif