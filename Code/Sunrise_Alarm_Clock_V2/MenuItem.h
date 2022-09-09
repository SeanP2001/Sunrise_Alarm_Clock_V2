
#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

enum functionIDs {
  EXIT, 
  BACK, 
  LIGHT_BAR_ON_TIME,
  LIGHT_BAR_OFF_TIME,
  BUZZER_ON_TIME,
  TURN_OFF_METHOD,
  USB_1_ON_TIME,
  USB_1_OFF_TIME,
  USB_2_ON_TIME,
  USB_2_OFF_TIME,
  USB_3_ON_TIME,
  USB_3_OFF_TIME,
  TIME_OFFSET,
  HOURS_BETWEEN_SYNCS,
  SYNC_TIME_NOW
};

class MenuItem{
public:
  int functionID = 0;

  String itemName = "";

  MenuItem* previousMenu;  // Pointer to Previous Menu
  MenuItem* subMenu;       // Pointer to first submenu item
  MenuItem* previousItem;  // Pointer to Previous Item
  MenuItem* nextItem;      // Pointer to Next Item

  MenuItem(String _itemName, int _functionID, MenuItem* _previousMenu, MenuItem* _previousItem);

  void printItemName();

  void addNewItem(String _itemName, int _functionID);

  void addNewSubMenu();
};

#endif // _MENU_ITEM_H_
