
#include "MenuItem.h"

MenuItem::MenuItem(String _itemName, int _functionID, MenuItem* _previousMenu, MenuItem* _previousItem)
{
  this->itemName = _itemName;
  this->functionID =_functionID;
  this->previousMenu = _previousMenu;
  this->previousItem = _previousItem;

  subMenu = NULL;
  nextItem = NULL;
}

void MenuItem::printItemName()
{
  display.print(itemName);
}

void MenuItem::executeFunction()
{
  display.clearDisplay(); 
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0); 
  if(previousMenu != NULL) 
  {
    display.print(previousMenu->itemName);
    display.print(">");
  }
  display.print(itemName); 
  display.display();
  
  switch (functionID)
  {
    case LIGHT_BAR_ON_TIME:
      break;
    case LIGHT_BAR_OFF_TIME:
      break;
    case BUZZER_ON_TIME:
      break;
    case TURN_OFF_METHOD:
      break;
    case USB_1_ON_TIME:
      break;
    case USB_1_OFF_TIME:
      break;
    case USB_2_ON_TIME:
      break;
    case USB_2_OFF_TIME:
      break;
    case USB_3_ON_TIME:
      break;
    case USB_3_OFF_TIME:
      break;
    case TIME_OFFSET:
      break;
    case HOURS_BETWEEN_SYNCS:
      break;
    case SYNC_TIME_NOW:
      break;
  }

  delay(3000);
}

void MenuItem::addNewItem(String _itemName, int _functionID)
{
  this->nextItem = new MenuItem(_itemName, _functionID, this->previousMenu, this);
}

void MenuItem::addNewSubMenu()
{
  this->subMenu = new MenuItem("Back", 1, this, NULL);
}
