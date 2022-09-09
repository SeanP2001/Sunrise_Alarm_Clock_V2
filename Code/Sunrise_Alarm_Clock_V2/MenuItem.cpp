
#include "MenuItem.h"

//-------------------------------------------- M E N U   I T E M   C O N S T R U C T O R --------------------------------------------

MenuItem::MenuItem(String _itemName, int _functionID, MenuItem* _previousMenu, MenuItem* _previousItem)
{
  this->itemName = _itemName;
  this->functionID =_functionID;
  this->previousMenu = _previousMenu;
  this->previousItem = _previousItem;

  subMenu = NULL;
  nextItem = NULL;
}

//-------------------------------------------------- P R I N T   I T E M   N A M E --------------------------------------------------

void MenuItem::printItemName()
{
  display.print(itemName);
}

//----------------------------------------------------- A D D   N E W   I T E M -----------------------------------------------------

void MenuItem::addNewItem(String _itemName, int _functionID)
{
  this->nextItem = new MenuItem(_itemName, _functionID, this->previousMenu, this);
}

//------------------------------------------------- A D D   N E W   S U B   M E N U -------------------------------------------------

void MenuItem::addNewSubMenu()
{
  this->subMenu = new MenuItem("Back", 1, this, NULL);
}
