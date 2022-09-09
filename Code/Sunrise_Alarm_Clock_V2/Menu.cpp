
#include "Menu.h"

//------------------------------------------------- M E N U   C O N S T R U C T O R -------------------------------------------------

Menu::Menu()
{ 
  this->firstItem = new MenuItem("Exit Menu", 0, NULL, NULL);    

  this->currentItem = firstItem;

  isOpen = false;
}

//------------------------------------------------------------- O P E N -------------------------------------------------------------

void Menu::open()
{
  isOpen = true;
  displayMenu();
}

//------------------------------------------------------------ C L O S E ------------------------------------------------------------

void Menu::close()
{
  isOpen = false;
  currentItem = firstItem;
  display.clearDisplay();                                
  display.display();
}

//----------------------------------------------------- D I S P L A Y   M E N U -----------------------------------------------------

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

//-------------------------------------------- M O V E   T O   P R E V I O U S   I T E M --------------------------------------------

void Menu::moveToPreviousItem()
{
  if(currentItem->previousItem != NULL)
  {
    currentItem = currentItem->previousItem;
  }  
}

//------------------------------------------------ M O V E   T O   N E X T   I T E M ------------------------------------------------
  
void Menu::moveToNextItem()
{
  if(currentItem->nextItem != NULL) 
  {
    currentItem = currentItem->nextItem;
  }
}

//------------------------------------------------------ S E L E C T   I T E M ------------------------------------------------------

int Menu::selectItem()
{
  if(currentItem->subMenu != NULL)                           // if the selected item has a sub-menu
  {
    currentItem = currentItem->subMenu;                      // navigate to it
  }
  else if(currentItem->functionID == EXIT)                   // if the selected item is to "Exit Menu"
  {
    close();                                                 // close the menu and return to the main screen
  } 
  else if(currentItem->functionID == BACK)                   // if the selected item is "Back"
  {
    currentItem = currentItem->previousMenu;                 // navigate to the previous menu
  }
  else                                                       // for all other function IDs
  {
    display.clearDisplay();                                  // Clear the display
    display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0); 
    if(currentItem->previousMenu != NULL)                    // and print "menu>setting" at the top of the display
    {
      display.print(currentItem->previousMenu->itemName);
      display.print(">");
    }
    display.print(currentItem->itemName); 
    display.display();
  }

  return currentItem->functionID;                            // return the function ID of the current item
}
