
#include "Menu.h"

Menu::Menu()
{ 
  this->firstItem = new MenuItem("Exit Menu", 0, NULL, NULL);    

  this->currentItem = firstItem;

  isOpen = false;
}

void Menu::open()
{
  isOpen = true;
  displayMenu();
}

void Menu::close()
{
  isOpen = false;
  currentItem = firstItem;
  display.clearDisplay();                                
  display.display();
}

void Menu::displayMenu()
{
  display.clearDisplay(); 
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if(currentItem->previousItem != NULL)            // if there is an menu item before the one currently selected
  {
    display.setCursor(12,8);                       
    currentItem->previousItem->printItemName();    // print it above the current item in the menu
  }

  display.setCursor(0,24);
  display.print(">");                              // print the cursor

  display.setCursor(12,24);
  currentItem->printItemName();
  
  if(currentItem->nextItem != NULL)                // if there is an item after the one currently selected
  {
    display.setCursor(12,40);
    currentItem->nextItem->printItemName();        // print it below the current item in the menu
  }

  display.display();
}

void Menu::moveToPreviousItem()
{
  if(currentItem->previousItem != NULL)
  {
    currentItem = currentItem->previousItem;
  }  
}
  
void Menu::moveToNextItem()
{
  if(currentItem->nextItem != NULL) 
  {
    currentItem = currentItem->nextItem;
  }
}

void Menu::selectItem()
{
  if(currentItem->subMenu != NULL)
  {
    currentItem = currentItem->subMenu;
  }
  else if(currentItem->functionID == EXIT)
  {
    close();
  }
  else if(currentItem->functionID == BACK)
  {
    currentItem = currentItem->previousMenu;
  }
  else
  {
    currentItem->executeFunction();
  }
}
