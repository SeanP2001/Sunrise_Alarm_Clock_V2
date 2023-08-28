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

#include "SettingsManager.h"
#include "DisplayManager.h"

#include "MenuItem.h"
#include "Menu.h"
#include "Stack.h"

#include "Light.h"
#include "USB.h"
#include "Alarm.h"

#include "WifiCredentials.h"

//----------------------------------------------------------- P I N O U T -----------------------------------------------------------

#define buttonsPin A0
#define lightBarPin D0
#define buzzerPin D5
#define usb1Pin D6
#define usb2Pin D7
#define usb3Pin D8

//---------------------------------------------------------- B U T T O N S ----------------------------------------------------------
                     //***** Button obj(Pin, AnalogValue, Tolerance) ******//
Button left(buttonsPin, 700, 100);          
Button middle(buttonsPin, 500, 100);
Button right(buttonsPin, 300, 100);

//---------------------------------------------------------- D E V I C E S ----------------------------------------------------------

Light lightBar(lightBarPin);
USB usb1(usb1Pin);
USB usb2(usb2Pin);
USB usb3(usb3Pin);

Alarm alarm(buzzerPin);

//------------------------------------------------ D I S P L A Y   V A R I A B L E S ------------------------------------------------  

#define SCREEN_WIDTH 128                                                    // OLED display width, in pixels
#define SCREEN_HEIGHT 64                                                    // OLED display height, in pixels
#define OLED_RESET     -1                                                   // Reset pin # (or -1 if sharing Arduino reset pin)
const uint8_t SCREEN_ADDRESS = 0x3C;                                        // See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   // Instantiate OLED Display

uint8_t screenTimeoutStart = 22;                                            // The screen will be able to timeout from 22:00
uint8_t screenTimeoutEnd = 7;                                               // to 07:00
uint8_t inactivityTimeout = 1;                                              // The duration of inactivity (in minutes) before the screen goes blank

bool screenIsOn = true;                                                     // turns the display on or off

//-------------------------------------------- T I M E   &   D A T E   V A R I A B L E S --------------------------------------------

uint8_t nextSyncHour = 0;                   // Variable to store the next hour when the clock needs to be synced

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

uint8_t ntpConnectionTimeout = 30;          // How many seconds before giving up connecting to the NTP server   


//----------------------------------------------- S E T T I N G S   V A R I A B L E S -----------------------------------------------

Settings currentSettings = {
    .lightBarOnTime = 0,
    .lightBarOffTime = 0,

    .buzzerOnTime = 0,
    .buzzerOffTime = 0,

    .usb1OnTime = 0,
    .usb1OffTime = 0,

    .usb2OnTime = 0,
    .usb2OffTime = 0,

    .usb3OnTime = 0,
    .usb3OffTime = 0,

    .utcOffset = 1,               // How many hours ahead or behind of UTC (BST=1 & GMT=0)
    .hrsBetweenSync = 12          // How often the clock needs to be synced (hours)
};

//------------------------------------------------------------- M E N U -------------------------------------------------------------

MenuItem* currentMenu = mainMenu;                                   // By default the main menu is selected as the current menu
uint8_t selectedIndex = 0;                                          // By default the first item on the menu is selected with the cursor

Stack parentMenuStack;                                              // A stack containing all of the parent menus (enables back navigation)

bool menuIsOpen = false;

enum Direction{LEFT, RIGHT};

//------------------------------------------------------------ S E T U P ------------------------------------------------------------

void setup()
{
  Serial.begin(115200);

  EEPROM.begin(512);

  loadSettings(currentSettings);                         // Load settings from EEPROM
  updateDevices(currentSettings);                        // Update all the device objects to reflect the current settings

  displayInit();                                         // Initialise the Display
  
  syncTime();                                            // Sync the local time to the NTP server
  calcNextSync(now(), currentSettings);                  // Calculate when the clock will next need to be synced

  if(itIsNight(now()))                                   // if it is night
  {
    resetScreenTimeout(now());                           // set the screen to timeout in 1 minute (default)
  }                   
}


//------------------------------------------------------------- M A I N -------------------------------------------------------------

void loop() 
{  
  manageScreenTimeout(now());                                 // Manage the screen timeout depending on the time of day
  manageOutputs(now());                                       // Turn on or off the light bar, buzzer or USB ports depending on the time

  if(screenIsOn)                                         // If the screen is on
  {
    displayTime(now());                                        // display the time
    displayIcons(menuIsOpen, lightBar.isOn(), alarm.isEnabled());  // and the icons

    if(left.buttonIsPressed())                             // Pressing the left button toggles the light bar on or off
    {
      lightBar.toggle();
      resetScreenTimeout(now());
    }

    if(middle.buttonIsPressed())                           // pressing the middle button enables and disables the buzzer 
    {      
      alarm.toggle();
      resetScreenTimeout(now());
    }

    if(right.buttonIsPressed())                            // Pressing the right button opens the menu
    {
      openMenu();
    }
  }
  else
  {
    displayBlankScreen();                                                           // make the screen blank 
  }



  while(menuIsOpen)                                           // When the menu is open
  {
    displayIcons(menuIsOpen, lightBar.isOn(), alarm.isEnabled());
    
    if(left.buttonIsPressed())                                // Pressing the left button moves the cursor to the previous item
    {
      navigateMenu(LEFT);
    }
    
    if(middle.buttonIsPressed())                              // Pressing the middle button selects the item highlighted by the cursor
    {
      handleMenuItem();
    }

    if(right.buttonIsPressed())                               // Pressing the right button moves the cursor to the next item (if there is one)
    {
      navigateMenu(RIGHT);
    }
    
    delay(100);
  }

  
  if(hour(now()) == nextSyncHour)                        // If the local clock needs to be synced
  {
    syncTime();                                          // Sync the local time to the NTP server

    nextSyncHour = calcNextSync(now(), currentSettings); // and calculate when the clock will next need to be synced
  }

  delay(100);
}

//-------------------------------------------------------- O P E N   M E N U --------------------------------------------------------
void openMenu()
{
  displayMenuSplashscreen();   
  delay(1000);                       
  menuIsOpen = true; 
  currentMenu = mainMenu;                              // By default the main menu is selected as the current menu
  selectedIndex = 0;                                   // By default the first item on the menu is selected with the cursor
  displayMenu(currentMenu, selectedIndex);
}
//------------------------------------------------- H A N D L E   M E N U   I T E M -------------------------------------------------
void handleMenuItem()
{
  enum FunctionID selectedSetting = currentMenu[selectedIndex].functionID;

  switch(selectedSetting)
  {
    case EXIT:
      closeMenu();
      break;

    case BACK:
      goToParentMenu();
      break;

    case SUBMENU:
      goToSubMenu();
      break;

    case SYNC_TIME_NOW:
      syncTime();                                               // Sync the local time to the NTP server
      calcNextSync(now(), currentSettings);                     // Calculate when the clock will next need to be synced
      break;

    case SAVE:
      saveSettingsAndDisplayMessage();
      break;

    default:
      displayBlankScreen();
      adjustSetting(selectedSetting, currentSettings);
      break;
  }

  if(menuIsOpen)                                          // If the menu is still open
  {
    displayMenu(currentMenu, selectedIndex);              // display the menu
  }
}


//---------------------------------------------------- N A V I G A T E   M E N U ----------------------------------------------------
void navigateMenu(enum Direction direction)
{
  if(direction == LEFT)
  {
    if(selectedIndex > 0)
    {
      selectedIndex--;
    }
  
    displayMenu(currentMenu, selectedIndex);
  }
  if(direction == RIGHT)
  {
    if(currentMenu[selectedIndex + 1].functionID != END_MARKER) 
    {
      selectedIndex++;
    }
  
    displayMenu(currentMenu, selectedIndex);
  }
}

//------------------------------------------------------- C L O S E   M E N U -------------------------------------------------------
void closeMenu()
{
  menuIsOpen = false;
  screenTimeoutMin = minute(now()) + 1;
  screenTimeoutSec = second(now());
  displayBlankScreen();
}

//------------------------------------------------ G O   T O   P A R E N T   M E N U ------------------------------------------------
void goToParentMenu()
{
  currentMenu = parentMenuStack.peek();
  parentMenuStack.pop();
  selectedIndex = 0;
}

//--------------------------------------------------- G O   T O   S U B   M E N U ---------------------------------------------------
void goToSubMenu()
{
  parentMenuStack.push(currentMenu);
  currentMenu = currentMenu[selectedIndex].subMenu;
  selectedIndex = 0;
}

//-------------------------------- S A V E   S E T T I N G S   A N D   D I S P L A Y   M E S S A G E --------------------------------
void saveSettingsAndDisplayMessage()
{
  displaySavingScreen();
  saveSettings(currentSettings);
  delay(1000);
  displaySavedScreen();
  delay(1000);
  displayBlankScreen();
}

//-------------------------------------------------------- S Y N C   T I M E --------------------------------------------------------

void syncTime()
{
  WiFi.mode(WIFI_STA);                              // Turn the WiFi on  
  WiFi.begin(ssid, password);                    

  display.clearDisplay();                           // Clear the Display
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
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

    String formattedDate = timeClient.getFormattedDate(); // FORMAT 2020-08-14T19:26:35Z

    String yearStr = formattedDate.substring(0, 4);       // Extract year and convert it to an int
    int year;
    year = yearStr.toInt();
      
    String monthStr = formattedDate.substring(5, 7);      // Extract month and convert it to an int
    int month;
    month = monthStr.toInt();

    String dayStr = formattedDate.substring(8, 10);       // Extract day and convert it into an int
    int day;
    day = dayStr.toInt();

    setTime(timeClient.getHours() + currentSettings.utcOffset, timeClient.getMinutes(), timeClient.getSeconds(), day, month, year); // Set the local time
  }
  else                                              // If the WiFi has failed to connect
  {
    display.println("Connection");                  // Show that the alarm has failed to connect to the WiFi
    display.println("Failed"); 
    display.display();
    delay(2000);
  }

  displayBlankScreen();
    
  WiFi.mode(WIFI_OFF);                              // Turn off the WiFi

}

//---------------------------------------------- C A L C U L A T E   N E X T   S Y N C ----------------------------------------------

uint8_t calcNextSync(time_t t, const Settings &settings)
{
  return (hour(t) + settings.hrsBetweenSync) % 24;      // Add to the current hour to find when the clock next needs to sync
}



//--------------------------------------------------- M A N A G E   O U T P U T S ---------------------------------------------------

void manageOutputs(time_t t)
{
  lightBar.manageOutput(t);         // Turn on the light bar when it is scheduled                    
  
  alarm.manageOutput(t);            // Sound the audio alarm when it is scheduled                
  
  usb1.manageOutput(t);             // Turn on USB port 1 when it is scheduled                     

  usb2.manageOutput(t);             // Turn on USB port 2 when it is scheduled                     

  usb3.manageOutput(t);             // Turn on USB port 3 when it is scheduled                     
}

//--------------------------------------------------- A D J U S T   S E T T I N G ---------------------------------------------------

void adjustSetting(FunctionID functionID, Settings &currentSettings) 
{
  delay(500);

  while (!middle.buttonIsPressed()) 
  {
    displaySetting(functionID, currentSettings);

    if (left.buttonIsPressed()) 
    {
      switch (functionID) {
        case LIGHT_BAR_ON_TIME:
          if (currentSettings.lightBarOnTime > 0) {
            currentSettings.lightBarOnTime--;
          }
          break;
        case LIGHT_BAR_OFF_TIME:
          if (currentSettings.lightBarOffTime > 0) {
            currentSettings.lightBarOffTime--;
          }
          break;
        case BUZZER_ON_TIME:
          if (currentSettings.buzzerOnTime > 0) {
            currentSettings.buzzerOnTime--;
          }
          break;
        case BUZZER_OFF_TIME:
          if (currentSettings.buzzerOffTime > 0) {
            currentSettings.buzzerOffTime--;
          }
          break;
        case USB_1_ON_TIME:
          if (currentSettings.usb1OnTime > 0) {
            currentSettings.usb1OnTime--;
          }
          break;
        case USB_1_OFF_TIME:
          if (currentSettings.usb1OffTime > 0) {
            currentSettings.usb1OffTime--;
          }
          break;
        case USB_2_ON_TIME:
          if (currentSettings.usb2OnTime > 0) {
            currentSettings.usb2OnTime--;
          }
          break;
        case USB_2_OFF_TIME:
          if (currentSettings.usb2OffTime > 0) {
            currentSettings.usb2OffTime--;
          }
          break;
        case USB_3_ON_TIME:
          if (currentSettings.usb3OnTime > 0) {
            currentSettings.usb3OnTime--;
          }
          break;
        case USB_3_OFF_TIME:
          if (currentSettings.usb3OffTime > 0) {
            currentSettings.usb3OffTime--;
          }
          break;
        case TIME_OFFSET:
          if (currentSettings.utcOffset > -12) {
            currentSettings.utcOffset--;
          }
          break;
        case HOURS_BETWEEN_SYNCS:
          if (currentSettings.hrsBetweenSync > 1) {
            currentSettings.hrsBetweenSync--;
          }
          break;
      }

      updateDevices(currentSettings);         // Update all the device objects to reflect the current settings
    }

    if (right.buttonIsPressed()) 
    {
      switch (functionID) {
        case LIGHT_BAR_ON_TIME:
          if (currentSettings.lightBarOnTime < 23) {
            currentSettings.lightBarOnTime++;
          }
          break;
        case LIGHT_BAR_OFF_TIME:
          if (currentSettings.lightBarOffTime < 23) {
            currentSettings.lightBarOffTime++;
          }
          break;
        case BUZZER_ON_TIME:
          if (currentSettings.buzzerOnTime < 23) {
            currentSettings.buzzerOnTime++;
          }
          break;
        case BUZZER_OFF_TIME:
          if (currentSettings.buzzerOffTime < 23) {
            currentSettings.buzzerOffTime++;
          }
          break;
        case USB_1_ON_TIME:
          if (currentSettings.usb1OnTime < 23) {
            currentSettings.usb1OnTime++;
          }
          break;
        case USB_1_OFF_TIME:
          if (currentSettings.usb1OffTime < 23) {
            currentSettings.usb1OffTime++;
          }
          break;
        case USB_2_ON_TIME:
          if (currentSettings.usb2OnTime < 23) {
            currentSettings.usb2OnTime++;
          }
          break;
        case USB_2_OFF_TIME:
          if (currentSettings.usb2OffTime < 23) {
            currentSettings.usb2OffTime++;
          }
          break;
        case USB_3_ON_TIME:
          if (currentSettings.usb3OnTime < 23) {
            currentSettings.usb3OnTime++;
          }
          break;
        case USB_3_OFF_TIME:
          if (currentSettings.usb3OffTime < 23) {
            currentSettings.usb3OffTime++;
          }
          break;
        case TIME_OFFSET:
          if (currentSettings.utcOffset < 12) {
            currentSettings.utcOffset++;
          }
          break;
        case HOURS_BETWEEN_SYNCS:
          if (currentSettings.hrsBetweenSync < 24) {
            currentSettings.hrsBetweenSync++;
          }
          break;
      }
      updateDevices(currentSettings);           // Update all the device objects to reflect the current settings
    }

    delay(100);
  }

  displayBlankScreen();                         // Clear the Display when the user exits the screen
}


//--------------------------------------------------- U P D A T E   D E V I C E S ---------------------------------------------------
void updateDevices(const Settings &settings)
{
  lightBar.setOnTime(settings.lightBarOnTime);          // Set the on time of the device based on the setting
  lightBar.setOffTime(settings.lightBarOffTime);

  usb1.setOnTime(settings.usb1OnTime);
  usb1.setOffTime(settings.usb1OffTime);

  usb2.setOnTime(settings.usb2OnTime);
  usb2.setOffTime(settings.usb2OffTime);

  usb3.setOnTime(settings.usb3OnTime);
  usb3.setOffTime(settings.usb3OffTime);

  alarm.setOnTime(settings.buzzerOnTime);
  alarm.setOffTime(settings.buzzerOffTime);
}

//------------------------------------------------------ I T   I S   N I G H T ------------------------------------------------------

bool itIsNight(time_t t)
{
  return (hour(t) >= screenTimeoutStart || hour(t) < screenTimeoutEnd);               // At night (22:00-07:00 by default)
}

//-------------------------------------------- A N Y   B U T T O N   I S   P R E S S E D --------------------------------------------

bool anyButtonIsPressed()
{
  return (left.buttonIsPressed() || middle.buttonIsPressed() || right.buttonIsPressed());
}
