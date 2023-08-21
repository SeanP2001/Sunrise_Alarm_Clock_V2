// DisplayManager.cpp

#include "DisplayManager.h"



//----------------------------------------------------- D I S P L A Y   I N I T -----------------------------------------------------
void displayInit()
{
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);   // Setup the OLED Display
  display.clearDisplay();                                
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
}


//----------------------------------------------------- D I S P L A Y   T I M E -----------------------------------------------------

void displayTime(time_t t)
{ 
  display.setTextSize(2); 
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,5);
  display.print("           ");

  switch(weekday(t))                                   // Centre the text
  {
    case 1: display.setCursor(28,5); break;
    case 2: display.setCursor(28,5); break;
    case 3: display.setCursor(22,5); break;
    case 4: display.setCursor(10,5); break;
    case 5: display.setCursor(16,5); break;
    case 6: display.setCursor(28,5); break;
    case 7: display.setCursor(16,5); break;
  }
 
  display.println(dayStr(weekday(t))); 

  display.setCursor(16,24);
  
  if (hour(t) < 10)
  {
    display.print("0");
  }
  display.print(hour(t));
  
  display.print(":");
  
  if (minute(t) < 10)
  {
    display.print("0");
  }
  display.print(minute(t));
  
  display.print(":");

  if (second(t) < 10)
  {
    display.print("0");
  }
  display.println(second(t));

  display.display();
}

//---------------------------------------- D I S P L A Y   M E N U   S P L A S H S C R E E N ----------------------------------------
void displayMenuSplashscreen()
{
  display.clearDisplay();                                
  display.display();
  display.setCursor(0,24);
  display.println(" Main Menu ");
  display.drawRect(0, 0, 128, 64, WHITE);    
  display.display();   
}


//----------------------------------------------------- D I S P L A Y   M E N U -----------------------------------------------------
void displayMenu(const MenuItem currentMenu[], uint8_t selectedIndex)
{
  display.clearDisplay(); 
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (selectedIndex > 0)                                                    // if there is an menu item before the one currently selected
  {
    display.setCursor(12,3);                       
    display.print(currentMenu[selectedIndex-1].itemName);                   // print it above the current item in the menu
  }

  display.setCursor(0,19);
  display.print(">");                                                       // print the cursor

  display.setCursor(12,19);
  display.print(currentMenu[selectedIndex].itemName);                       // print the current menu item

  if (currentMenu[selectedIndex + 1].functionID != END_MARKER)              // if there is an menu item after the one currently selected
  {
    display.setCursor(12,35);                       
    display.print(currentMenu[selectedIndex+1].itemName);                   // print it below the current item in the menu
  }
}


//-------------------------------------------------- D I S P L A Y   S E T T I N G --------------------------------------------------

void displaySetting(const FunctionID functionID, const Settings &settings)
{  
  display.setTextSize(2); 
  display.setTextColor(WHITE, BLACK);

  display.drawBitmap(2, 48, downArrowIcon, 16, 16, WHITE);
  display.drawBitmap(110, 48, upArrowIcon, 16, 16, WHITE);
  
  display.setCursor(0,16);
  display.print("           ");
  display.setCursor(0,16);
  
  switch (functionID)
  {
    case LIGHT_BAR_ON_TIME:
      if (settings.lightBarOnTime < 10)                                // display the on time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.lightBarOnTime);
      display.print(":00");
      break;
      
    case LIGHT_BAR_OFF_TIME:
      if (settings.lightBarOffTime < 10)                               // display the off time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.lightBarOffTime);
      display.print(":00");
      break;
      
    case BUZZER_ON_TIME:
      if (settings.buzzerOnTime < 10)                                  // display the on time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.buzzerOnTime);
      display.print(":00");
      break;

    case BUZZER_OFF_TIME:
      if (settings.buzzerOffTime < 10)                                 // display the off time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.buzzerOffTime);
      display.print(":00");
      break;
      
    case USB_1_ON_TIME:
      if (settings.usb1OnTime < 10)                                    // display the on time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.usb1OnTime);
      display.print(":00");
      break;
      
    case USB_1_OFF_TIME:
      if (settings.usb1OffTime < 10)                                   // display the off time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.usb1OffTime);
      display.print(":00");
      break;
      
    case USB_2_ON_TIME:
      if (settings.usb2OnTime < 10)                                    // display the on time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.usb2OnTime);
      display.print(":00");
      break;
      
    case USB_2_OFF_TIME:
      if (settings.usb2OffTime < 10)                                   // display the off time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.usb2OffTime);
      display.print(":00");
      break;
      
    case USB_3_ON_TIME:
      if (settings.usb3OnTime < 10)                                    // display the on time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.usb3OnTime);
      display.print(":00");
      break;
      
    case USB_3_OFF_TIME:
      if (settings.usb3OffTime < 10)                                   // display the off time in the format HH:00
      {
        display.print("0");
      }
      display.print(settings.usb3OffTime);
      display.print(":00");
      break;
      
    case TIME_OFFSET:
      display.print(settings.utcOffset);                 // print the number of hours offset from UTC
      display.print(" Hours");
      break;
      
    case HOURS_BETWEEN_SYNCS:      
      display.print(settings.hrsBetweenSync);                          // print the number of hours between syncs
      display.print(" Hours");            
      break;  
  }
  
  display.display();
}

//---------------------------------------------------- D I S P L A Y   I C O N S ----------------------------------------------------
void displayIcons(bool menuState, bool lightBarState, bool buzzerEnabled)
{
  if(menuState == 1)
  {
    display.drawBitmap(2, 48, upArrowIcon, 16, 16, WHITE);
    display.drawBitmap(110, 48, downArrowIcon, 16, 16, WHITE);
  }
  else
  {
    if(lightBarState == 1)
    {
      display.drawBitmap(2, 48, lightOnIcon, 16, 16, WHITE);
    }
    else
    {
      display.setCursor(0,48);
      display.print("  ");
      display.drawBitmap(2, 48, lightOffIcon, 16, 16, WHITE);
    }
  
    if(buzzerEnabled)
    {
      display.drawBitmap(56, 48, alarmOnIcon, 16, 16, WHITE);
    }
    else
    {
      display.setCursor(56,48);
      display.print("  ");
      display.drawBitmap(56, 48, alarmOffIcon, 16, 16, WHITE);
    }
  
    display.drawBitmap(110, 48, menuIcon, 16, 16, WHITE);
  }
    
  display.display();
}


//-------------------------------------------- D I S P L A Y   S A V I N G   S C R E E N --------------------------------------------
void displaySavingScreen()
{
  display.clearDisplay();                                   // Clear the Display
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,0);
  display.print("Saving...");
  display.display();
}


//--------------------------------------------- D I S P L A Y   S A V E D   S C R E E N ---------------------------------------------
void displaySavedScreen()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,0);
  display.print("Saved");
  display.display();
}