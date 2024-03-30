#ifndef MENU_H
#define MENU_H

struct State;
struct Settings;
class SSD1306AsciiWire;

void initMenu();

void updateMenu(Settings& settings, State& state);

#endif