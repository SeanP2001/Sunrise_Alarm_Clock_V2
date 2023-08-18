
#ifndef MENU_ITEM_H
#define MENU_ITEM_H


enum FunctionID {
  EXIT, 
  BACK, 
  SUBMENU,
  LIGHT_BAR_ON_TIME,
  LIGHT_BAR_OFF_TIME,
  BUZZER_ON_TIME,
  BUZZER_OFF_TIME,
  USB_1_ON_TIME,
  USB_1_OFF_TIME,
  USB_2_ON_TIME,
  USB_2_OFF_TIME,
  USB_3_ON_TIME,
  USB_3_OFF_TIME,
  TIME_OFFSET,
  HOURS_BETWEEN_SYNCS,
  SYNC_TIME_NOW,
  SAVE,
  END_MARKER
};

struct MenuItem {
  char itemName[20];
  FunctionID functionID;
  MenuItem* subMenu;
};

#endif // MENU_ITEM_H
