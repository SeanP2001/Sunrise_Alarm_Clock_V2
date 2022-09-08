
#ifndef _MENU_H_
#define _MENU_H_

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#include "MenuItem.h"

extern Adafruit_SSD1306 display;

class Menu{
public:
  
  MenuItem* firstItem;    // Pointer to First Menu Item

  MenuItem* currentItem;  // Pointer to Current Menu Item

  bool isOpen;

  Menu();

  void open();

  void close();

  void displayMenu();

  void moveToPreviousItem();
  
  void moveToNextItem();

  void selectItem();
};

#endif // _MENU_H_
