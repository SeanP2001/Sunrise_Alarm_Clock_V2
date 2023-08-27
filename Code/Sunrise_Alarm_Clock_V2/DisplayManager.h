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

extern uint8_t screenTimeoutStart;
extern uint8_t screenTimeoutEnd;
extern uint8_t inactivityTimeout;

// uint8_t screenTimeoutSec = 0;                                               // variable to store the second when the screen will time out 
// uint8_t screenTimeoutMin = 0;                                               // variable to store the minute when the screen will time out 

extern uint8_t screenTimeoutSec;          // variable to store the second when the screen will time out 
extern uint8_t screenTimeoutMin;          // variable to store the minute when the screen will time out 

extern bool itIsNight(time_t t);
extern bool anyButtonIsPressed();

void displayInit();
void displayTime(time_t t);
void displayMenuSplashscreen();
void displayMenu(const MenuItem currentMenu[], uint8_t selectedIndex);
void displaySetting(const FunctionID functionID, const Settings &settings);
void displayBlankScreen();

void displayIcons(bool menuState, bool lightBarState, bool buzzerEnabled);

void displaySavingScreen();
void displaySavedScreen();

void resetScreenTimeout(time_t t);
void manageScreenTimeout(time_t t);

#endif // DISPLAY_MANAGER_H