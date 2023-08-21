// DisplayManager.h

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_SSD1306.h>
#include <TimeLib.h>

#include "Icons.h"
#include "MenuItem.h"
#include "SettingsManager.h"

extern const uint8_t SCREEN_ADDRESS;
extern Adafruit_SSD1306 display;
extern bool screenIsOn;

void displayInit();
void displayTime(time_t t);
void displayMenuSplashscreen();
void displayMenu(const MenuItem currentMenu[], uint8_t selectedIndex);
void displaySetting(const FunctionID functionID, const Settings &settings);

void displayIcons(bool menuState, bool lightBarState, bool buzzerEnabled);

void displaySavingScreen();
void displaySavedScreen();

#endif // DISPLAY_MANAGER_H