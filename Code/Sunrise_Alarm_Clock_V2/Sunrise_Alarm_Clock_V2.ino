//-----------------------------------------------------------------------------------------------------------------------------------
// Sunrise Alarm Clock V2
// Sean Price
//-----------------------------------------------------------------------------------------------------------------------------------

#include <TimeLib.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Button.h"
#include "Menu.h"
#include "MenuItem.h"

//----------------------------------------------------------- P I N O U T -----------------------------------------------------------

#define buttonsPin A0
#define ledBarPin D0
#define buzzerPin D5
#define usb1Pin D6
#define usb2Pin D7
#define usb3Pin D8

                     //***** Button obj(Pin, AnalogValue, Tolerance) ******//
Button left(buttonsPin, 700, 100);          
Button middle(buttonsPin, 500, 100);
Button right(buttonsPin, 300, 100);

//------------------------------------------------ D I S P L A Y   V A R I A B L E S ------------------------------------------------  

#define SCREEN_WIDTH 128                                                    // OLED display width, in pixels
#define SCREEN_HEIGHT 64                                                    // OLED display height, in pixels

#define OLED_RESET     -1                                                   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c                                                 // See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   // Instantiate OLED Display

//------------------------------------------------- W I F I   C R E D E N T I A L S -------------------------------------------------  

const char *ssid     = "YOUR_WIFI_SSID";        // REPLACE WITH YOUR WIFI SSID
const char *password = "YOUR_WIFI_PASSWORD";    // REPLACE WITH YOUR WIFI PASSWORD

//-------------------------------------------- T I M E   &   D A T E   V A R I A B L E S --------------------------------------------

long utcOffsetInSeconds = 3600;             // BST = (3600 secs) ahead of UTC and GMT= (0 secs) ahead of UTC

char daysOfTheWeek[7][12] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

int nextSyncHour = 0;                       // Variable to store the next hour when the clock needs to be synced
int hrsBetweenSync = 12;                    // How often the clock needs to be synced (hours) 

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

int ntpConnectionTimeout = 30;              // How many seconds before giving up connecting to the NTP server   

//------------------------------------------------ G E N E R A L   V A R I A B L E S ------------------------------------------------

bool ledBarState = 0;

Menu mainMenu;                                            // Instantiates a menu called mainMenu

//----------------------------------------------- S E T T I N G S   V A R I A B L E S -----------------------------------------------

int lightBarOnTime = 0;
int lightBarOffTime = 0;

int buzzerOnTime = 0;
int buzzerOffTime = 0;

int usb1OnTime = 0;
int usb1OffTime = 0;

int usb2OnTime = 0;
int usb2OffTime = 0;

int usb3OnTime = 0;
int usb3OffTime = 0;



//------------------------------------------------------------ S E T U P ------------------------------------------------------------

void setup(){
  Serial.begin(115200);

  pinMode(ledBarPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(usb1Pin, OUTPUT);
  pinMode(usb2Pin, OUTPUT);
  pinMode(usb3Pin, OUTPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);   // Setup the OLED Display
  display.clearDisplay();                                
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  syncTime();                                            // Sync the local time to the NTP server
  calcNextSync();                                        // Calculate when the clock will next need to be synced

  buildMenu(mainMenu);                                   // build the mainMenu
}

//------------------------------------------------------------- M A I N -------------------------------------------------------------

void loop() 
{
  displayTime(now());                                    // Display the time

  if(left.buttonIsPressed())                             // Pressing the left button toggles the led bar on or off
  {
    if(ledBarState == 0) 
    {
      digitalWrite(ledBarPin, HIGH);
      ledBarState = 1;
    }
    else
    {
      digitalWrite(ledBarPin, LOW);
      ledBarState = 0;
    }
  }

  if(right.buttonIsPressed())                             // Pressing the right button opens the menu
  {
    display.clearDisplay();                                
    display.display();
    display.setCursor(0,0);
    display.print("Main Menu");
    display.display();      
    delay(1000);                       
    mainMenu.open(); 
  }

  while(mainMenu.isOpen)                                  // When the menu is open
  {
    if(left.buttonIsPressed())                            // Pressing the left button moves the cursor to the previous item
    {
      mainMenu.moveToPreviousItem();
      mainMenu.displayMenu();
    }
    
    if(middle.buttonIsPressed())                          // Pressing the middle button selects the item highlighted by the cursor
    {
      int selectedSetting = mainMenu.selectItem();

      if(selectedSetting > 1 && selectedSetting != NULL)
      {
        adjustSetting(selectedSetting);
      }
    
      if(mainMenu.isOpen)
      {
        mainMenu.displayMenu();
      }      
    }

    if(right.buttonIsPressed())                           // Pressing the right button moves the cursor to the next item  
    {
      mainMenu.moveToNextItem();
      mainMenu.displayMenu();
    }

    delay(100);
  }
  
  if(hour(now()) == nextSyncHour)                        // If the local clock needs to be synced
  {
    syncTime();                                          // Sync the local time to the NTP server

    calcNextSync();                                      // and calculate when the clock will next need to be synced
  }
}

//-------------------------------------------------------- S Y N C   T I M E --------------------------------------------------------

void syncTime()
{
  WiFi.mode(WIFI_STA);                              // Turn the WiFi on  
  WiFi.begin(ssid, password);                    

  display.clearDisplay();                           // Clear the Display
  display.setCursor(0,0);
  display.println("Connecting");                    // Show that the device is connecting to the WiFi
  display.display();


  for ( int sec = 0; (WiFi.status() != WL_CONNECTED) && (sec < ntpConnectionTimeout); sec++)  // Until the WiFi Connects or times out
  {  
    display.print(".");                             // Put dots on the screen to show it hasn't frozen
    display.display();
    delay (1000);
  }

  display.clearDisplay();                           // Clear the Display 
  display.setCursor(0,0);

  if(WiFi.status() == WL_CONNECTED)                 // If the WiFi has successfully connected
  {
    display.println("Connected");                   // Show that the alarm has connected to the WiFi
    display.display();
    delay(2000);

    timeClient.begin();                             // Connect to the NTP Server
    timeClient.update();                            // Get the time from the NTP Server

    setTime(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds(), (timeClient.getDay() -1),0,0); // Set the local time
  }
  else                                              // If the WiFi has failed to connect
  {
    display.println("Connection");                  // Show that the alarm has failed to connect to the WiFi
    display.println("Failed"); 
    display.display();
    delay(2000);
  }

  display.clearDisplay(); 
  display.display();
    
  WiFi.mode(WIFI_OFF);                              // Turn off the WiFi

}

//---------------------------------------------- C A L C U L A T E   N E X T   S Y N C ----------------------------------------------

void calcNextSync()
{
  nextSyncHour = hour(now()) + hrsBetweenSync;       // Add to the current hour to find when the clock next needs to sync

  while(nextSyncHour >= 24)                       
  {
    nextSyncHour = nextSyncHour - 24;             
  }
}

//----------------------------------------------------- D I S P L A Y   T I M E -----------------------------------------------------

void displayTime(time_t t)
{
  display.clearDisplay(); 
  display.setTextSize(2); 
  display.setCursor(0,16);

  display.println(daysOfTheWeek[day(t)]);           
  
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

//--------------------------------------------------- A D J U S T   S E T T I N G ---------------------------------------------------

void adjustSetting(int functionID)
{
  delay(500);
  
  display.setTextSize(2); 
  display.setTextColor(WHITE, BLACK);
  
  
  while(!middle.buttonIsPressed())                              // pressing the middle button exits back to the menu (confirming setting)
  {
    display.setCursor(0,16);
    
    switch (functionID)
    {
      case LIGHT_BAR_ON_TIME:
        if (lightBarOnTime < 10)                                // display the on time in the format HH:00
        {
          display.print("0");
        }
        display.print(lightBarOnTime);
        display.print(":00");

        if(left.buttonIsPressed() && lightBarOnTime >= 0)       // press the left button to decrement the on time (down to 00:00)
        {
          lightBarOnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && lightBarOnTime < 24)      // press the right button to increment the on time (up to 23:00)
        {
          lightBarOnTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case LIGHT_BAR_OFF_TIME:
        if (lightBarOffTime < 10)                               // display the off time in the format HH:00
        {
          display.print("0");
        }
        display.print(lightBarOffTime);
        display.print(":00");

        if(left.buttonIsPressed() && lightBarOffTime >= 0)      // press the left button to decrement the off time (down to 00:00)
        {
          lightBarOffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && lightBarOffTime < 24)     // press the right button to increment the off time (up to 23:00)
        {
          lightBarOffTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case BUZZER_ON_TIME:
        if (buzzerOnTime < 10)                                  // display the on time in the format HH:00
        {
          display.print("0");
        }
        display.print(buzzerOnTime);
        display.print(":00");

        if(left.buttonIsPressed() && buzzerOnTime >= 0)         // press the left button to decrement the on time (down to 00:00)
        {
          buzzerOnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && buzzerOnTime < 24)        // press the right button to increment the on time (up to 23:00)
        {
          buzzerOnTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;

      case BUZZER_OFF_TIME:
        if (buzzerOffTime < 10)                                 // display the off time in the format HH:00
        {
          display.print("0");
        }
        display.print(buzzerOffTime);
        display.print(":00");

        if(left.buttonIsPressed() && buzzerOffTime >= 0)        // press the left button to decrement the off time (down to 00:00)
        {
          buzzerOffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && buzzerOffTime < 24)       // press the right button to increment the off time (up to 23:00)
        {
          buzzerOffTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case USB_1_ON_TIME:
        if (usb1OnTime < 10)                                    // display the on time in the format HH:00
        {
          display.print("0");
        }
        display.print(usb1OnTime);
        display.print(":00");

        if(left.buttonIsPressed() && usb1OnTime >= 0)           // press the left button to decrement the on time (down to 00:00)
        {
          usb1OnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb1OnTime < 24)          // press the right button to increment the on time (up to 23:00)
        {
          usb1OnTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case USB_1_OFF_TIME:
        if (usb1OffTime < 10)                                   // display the off time in the format HH:00
        {
          display.print("0");
        }
        display.print(usb1OffTime);
        display.print(":00");

        if(left.buttonIsPressed() && usb1OffTime >= 0)          // press the left button to decrement the off time (down to 00:00)
        {
          usb1OffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb1OffTime < 24)         // press the right button to increment the off time (up to 23:00)
        {
          usb1OffTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case USB_2_ON_TIME:
        if (usb2OnTime < 10)                                    // display the on time in the format HH:00
        {
          display.print("0");
        }
        display.print(usb2OnTime);
        display.print(":00");

        if(left.buttonIsPressed() && usb2OnTime >= 0)           // press the left button to decrement the on time (down to 00:00)
        {
          usb2OnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb2OnTime < 24)          // press the right button to increment the on time (up to 23:00)
        {
          usb2OnTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case USB_2_OFF_TIME:
        if (usb2OffTime < 10)                                   // display the off time in the format HH:00
        {
          display.print("0");
        }
        display.print(usb2OffTime);
        display.print(":00");

        if(left.buttonIsPressed() && usb2OffTime >= 0)          // press the left button to decrement the off time (down to 00:00)
        {
          usb2OffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb2OffTime < 24)         // press the right button to increment the off time (up to 23:00)
        {
          usb2OffTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case USB_3_ON_TIME:
        if (usb3OnTime < 10)                                    // display the on time in the format HH:00
        {
          display.print("0");
        }
        display.print(usb3OnTime);
        display.print(":00");

        if(left.buttonIsPressed() && usb3OnTime >= 0)           // press the left button to decrement the on time (down to 00:00)
        {
          usb3OnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb3OnTime < 24)          // press the right button to increment the on time (up to 23:00)
        {
          usb3OnTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case USB_3_OFF_TIME:
        if (usb3OffTime < 10)                                   // display the off time in the format HH:00
        {
          display.print("0");
        }
        display.print(usb3OffTime);
        display.print(":00");

        if(left.buttonIsPressed() && usb3OffTime >= 0)          // press the left button to decrement the off time (down to 00:00)
        {
          usb3OffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb3OffTime < 24)         // press the right button to increment the off time (up to 23:00)
        {
          usb3OffTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case TIME_OFFSET:
        display.print(utcOffsetInSeconds/3600);                 // print the number of hours offset from UTC
        display.print(" Hours");

        if(left.buttonIsPressed() && hrsBetweenSync >= -12)     // press the left button to decrement the offset time (down to -12)
        {
          utcOffsetInSeconds = utcOffsetInSeconds - 3600; 
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && hrsBetweenSync <= 12)     // press the right button to increment the offset time (up to +12)
        {
          utcOffsetInSeconds = utcOffsetInSeconds + 3600; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case HOURS_BETWEEN_SYNCS:      
        display.print(hrsBetweenSync);                          // print the number of hours between syncs
        display.print(" Hours");

        if(left.buttonIsPressed() && hrsBetweenSync > 0)        // press the left button to decrement the number of hours (down to 1) 
        {
          hrsBetweenSync--; 
          display.setCursor(0,16);
          display.print("           ");        
        } 
        if(right.buttonIsPressed() && hrsBetweenSync <= 24)     // press the right button to increment the number of hours (up to 24)
        {
          hrsBetweenSync++; 
          display.setCursor(0,16);
          display.print("           ");
        }               
        break;  
             
      case SYNC_TIME_NOW:
        syncTime();                                             // Sync the local time to the NTP server
        calcNextSync();                                         // Calculate when the clock will next need to be synced
        return;
    }
    
    display.display();
    delay(100);
  }
  
}
//------------------------------------------------------- B U I L D   M E N U -------------------------------------------------------
/*
  The following function will build this menu:

  1. Alarm
    1.1. Light Bar
      1.1.1. On Time
      1.1.2. Off Time
    1.2. Buzzer
      1.2.1. On Time
      1.2.2. Off Time
    
  2. USB Ports
    2.1. USB 1
      2.1.1. On Time
      2.1.2. Off Time
    2.2. USB 2
      2.2.1. On Time
      2.2.2. Off Time
    2.3. USB 3
      2.3.1. On Time
      2.3.2. Off Time

  3. Time
    3.1. Time Offset (hrs)
    3.2. Hours Between Syncs
    3.3. Sync Time Now
*/



void buildMenu(Menu menu)
{
  MenuItem* currentItem = menu.firstItem;                               // Create a pointer which points to the first item on the menu,

  currentItem->addNewItem("Alarm", NULL);                               // and add the next item with the name "Alarm" (This will link to a sub-menu so the functionID is NULL).
  currentItem = currentItem->nextItem;                                  // Point to the newly created item "Alarm",
  currentItem->addNewSubMenu();                                         // add a sub-menu to the "Alarm" item,
  currentItem = currentItem->subMenu;                                   // and point to the new sub-menu.

  currentItem->addNewItem("Light Bar", NULL);                           // In the new sub-menu, add an item with the name "Light Bar" (Again this will link to a sub-menu so the functionID is NULL).
  currentItem = currentItem->nextItem;                                  // Point to the newly created item "Light Bar"
  currentItem->addNewSubMenu();                                         // add a sub-menu to the "Light Bar" item,
  currentItem = currentItem->subMenu;                                   // and point to the new sub-menu. 

  currentItem->addNewItem("On Time", LIGHT_BAR_ON_TIME);                // In the new sub-menu, add an item with the name "On Time" (see "MenuItem.h" for functionID enum).
  currentItem = currentItem->nextItem;                                  // Point to the newly created item "On Time".

  currentItem->addNewItem("Off Time", LIGHT_BAR_OFF_TIME);              // Add a new item with the name "Off Time" (see "MenuItem.h" for functionID enum).

  currentItem = currentItem->previousMenu;                              // Point back to the previous menu (now pointing at the "Light Bar" item).

  currentItem->addNewItem("Buzzer", NULL);                              // Add an item with the name "Buzzer" (Again this will link to a sub-menu so the functionID is NULL).
  currentItem = currentItem->nextItem;                                  // Point to the newly created item "Buzzer",
  currentItem->addNewSubMenu();                                         // add a sub-menu to the "Buzzer" item,
  currentItem = currentItem->subMenu;                                   // and point to the new sub-menu.
 
  currentItem->addNewItem("On Time", BUZZER_ON_TIME);                   // In the new sub-menu, add an item with the name "On Time" (see "MenuItem.h" for functionID enum).
  currentItem = currentItem->nextItem;                                  // Point to the newly created item "On Time"

  currentItem->addNewItem("Off Time", BUZZER_OFF_TIME);                 // Add a new item with the name "Off Time" (see "MenuItem.h" for functionID enum).

  currentItem = currentItem->previousMenu;                              // Point back to the previous menu (now pointing at the "Buzzer" item).

  currentItem = currentItem->previousMenu;                              // Point back to the previous menu (now pointing at the "Alarm" item).

  currentItem->addNewItem("USB Ports", NULL);
  currentItem = currentItem->nextItem;
  currentItem->addNewSubMenu();
  currentItem = currentItem->subMenu;

  currentItem->addNewItem("USB 1", NULL);
  currentItem = currentItem->nextItem;
  currentItem->addNewSubMenu();
  currentItem = currentItem->subMenu;

  currentItem->addNewItem("On Time", USB_1_ON_TIME);
  
  currentItem = currentItem->nextItem;
  
  currentItem->addNewItem("Off Time", USB_1_OFF_TIME);

  currentItem = currentItem->previousMenu;

  currentItem->addNewItem("USB 2", NULL);
  currentItem = currentItem->nextItem;
  currentItem->addNewSubMenu();
  currentItem = currentItem->subMenu;

  currentItem->addNewItem("On Time", USB_2_ON_TIME);
  
  currentItem = currentItem->nextItem;
  
  currentItem->addNewItem("Off Time", USB_2_OFF_TIME);

  currentItem = currentItem->previousMenu;

  currentItem->addNewItem("USB 3", NULL);
  currentItem = currentItem->nextItem;
  currentItem->addNewSubMenu();
  currentItem = currentItem->subMenu;

  currentItem->addNewItem("On Time", USB_3_ON_TIME);
  
  currentItem = currentItem->nextItem;
  
  currentItem->addNewItem("Off Time", USB_3_OFF_TIME);

  currentItem = currentItem->previousMenu;

  currentItem = currentItem->previousMenu;

  currentItem->addNewItem("Time", NULL);
  currentItem = currentItem->nextItem;
  currentItem->addNewSubMenu();
  currentItem = currentItem->subMenu;

  currentItem->addNewItem("Time Offset (hrs)", TIME_OFFSET);

  currentItem = currentItem->nextItem;

  currentItem->addNewItem("Hours Between Syncs", HOURS_BETWEEN_SYNCS);

  currentItem = currentItem->nextItem;

  currentItem->addNewItem("Sync Time Now", SYNC_TIME_NOW);

  free(currentItem);                                                    // Delete the currentItem pointer (no longer needed).
}
