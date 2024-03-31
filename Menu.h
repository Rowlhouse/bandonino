#ifndef MENU_H
#define MENU_H

#include <SSD1306AsciiWire.h>

struct State;
struct Settings;

extern SSD1306AsciiWire oled;

void initMenu();

void updateMenu(Settings& settings, State& state);

void forceMenuRefresh();

#endif