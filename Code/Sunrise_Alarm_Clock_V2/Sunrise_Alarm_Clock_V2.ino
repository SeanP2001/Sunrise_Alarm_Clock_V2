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
#include "Device.h"

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

Device lightBar(lightBarPin);
Device usb1(usb1Pin);
Device usb2(usb2Pin);
Device usb3(usb3Pin);

//------------------------------------------------ D I S P L A Y   V A R I A B L E S ------------------------------------------------  

#define SCREEN_WIDTH 128                                                    // OLED display width, in pixels
#define SCREEN_HEIGHT 64                                                    // OLED display height, in pixels

#define OLED_RESET     -1                                                   // Reset pin # (or -1 if sharing Arduino reset pin)
const uint8_t SCREEN_ADDRESS = 0x3C;                                        // See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   // Instantiate OLED Display

bool screenIsOn = true;                                                     // turns the display on or off

uint8_t screenTimeoutStart = 22;                                            // The screen will be able to timeout from 22:00
uint8_t screenTimeoutEnd = 7;                                               // to 07:00

uint8_t screenTimeoutSec = 0;                                               // variable to store the second when the screen will time out 
uint8_t screenTimeoutMin = 0;                                               // variable to store the minute when the screen will time out 


//------------------------------------------------- W I F I   C R E D E N T I A L S -------------------------------------------------  

const char *ssid     = "YOUR_WIFI_SSID";        // REPLACE WITH YOUR WIFI SSID
const char *password = "YOUR_WIFI_PASSWORD";    // REPLACE WITH YOUR WIFI PASSWORD

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

bool buzzerEnabled = true;

//------------------------------------------------------------- M E N U -------------------------------------------------------------

MenuItem* currentMenu = mainMenu;                                   // By default the main menu is selected as the current menu
uint8_t selectedIndex = 0;                                          // By default the first item on the menu is selected with the cursor

Stack parentMenuStack;                                              // A stack containing all of the parent menus (enables back navigation)

bool menuIsOpen = false;

//------------------------------------------------------------ S E T U P ------------------------------------------------------------

void setup()
{
  Serial.begin(115200);

  EEPROM.begin(512);

  loadSettings(currentSettings);                         // Load settings from EEPROM
  updateDevices(currentSettings);                        // Update all the device objects to reflect the current settings

  pinMode(buzzerPin, OUTPUT);

  displayInit();                                         // Initialise the Display
  
  syncTime();                                            // Sync the local time to the NTP server
  calcNextSync();                                        // Calculate when the clock will next need to be synced

  if(itIsNight())                                        // if it is night
  {
    screenTimeoutMin = minute(now()) + 1;                // set the screen to timeout 1 minute after turning on
    screenTimeoutSec = second(now());   
  }                   
}


//------------------------------------------------------------- M A I N -------------------------------------------------------------

void loop() 
{  
  manageScreenTimeout();                                 // Manage the screen timeout depending on the time of day

  if(screenIsOn)                                         // If the screen is on
  {
    displayTime(now());                                        // display the time
    displayIcons(menuIsOpen, lightBar.isOn(), buzzerEnabled);  // and the icons
  }

  
  if(left.buttonIsPressed())                             // Pressing the left button toggles the light bar on or off
  {
    lightBar.toggle();

    screenTimeoutMin = minute(now()) + 1;                // stop the screen from timing out for another minute
    screenTimeoutSec = second(now());
  }


  if(middle.buttonIsPressed())                           // pressing the middle button enables and disables the buzzer 
  {      
    if(!buzzerEnabled) 
    {
      buzzerEnabled = true;
    }
    else
    {
      buzzerEnabled = false;
    }

    screenTimeoutMin = minute(now()) + 1;                // stop the screen from timing out for another minute
    screenTimeoutSec = second(now());
  }


  if(right.buttonIsPressed())                            // Pressing the right button opens the menu
  {
    displayMenuSplashscreen();   
    delay(1000);                       
    menuIsOpen = true; 
    currentMenu = mainMenu;                                     // By default the main menu is selected as the current menu
    selectedIndex = 0;                                          // By default the first item on the menu is selected with the cursor
    displayMenu(currentMenu, selectedIndex);
  }



  while(menuIsOpen)                                      // When the menu is open
  {
    displayIcons(menuIsOpen, lightBar.isOn(), buzzerEnabled);
    
    if(left.buttonIsPressed())                           // Pressing the left button moves the cursor to the previous item
    {
      if(selectedIndex > 0)
      {
        selectedIndex--;
      }
    
      displayMenu(currentMenu, selectedIndex);
    }
    
    if(middle.buttonIsPressed())                         // Pressing the middle button selects the item highlighted by the cursor
    {
      enum FunctionID selectedSetting = currentMenu[selectedIndex].functionID;

      switch(selectedSetting)
      {
        case EXIT:
          menuIsOpen = false;                           // Close the menu
          screenTimeoutMin = minute(now()) + 1;         // set the screen to timeout in a minute
          screenTimeoutSec = second(now());    
          display.clearDisplay();                                
          display.display();
          break;

        case BACK:
          currentMenu = parentMenuStack.peek();                     // Set the current menu to it's parent menu
          parentMenuStack.pop();                                    // remove the menu from the stack
          selectedIndex = 0;                                        // Select the first item on the menu
          break;

        case SUBMENU:
          parentMenuStack.push(currentMenu);                        // Add the current menu to the stack (so it can be navigated back to)
          currentMenu = currentMenu[selectedIndex].subMenu;         // Change the current menu to the submenu pointed to by the selected item
          selectedIndex = 0;                                        // Select the first item on the submenu
          break;

        case SYNC_TIME_NOW:
          syncTime();                                               // Sync the local time to the NTP server
          calcNextSync();                                           // Calculate when the clock will next need to be synced
          break;

        case SAVE:
          displaySavingScreen();
          
          saveSettings(currentSettings);                            // save the current settings to EEPROM
          
          delay(1000);
          
          displaySavedScreen();
          delay(1000);
          
          display.clearDisplay();
          display.display();
          break;

        default:
          display.clearDisplay();
          display.display();
          adjustSetting(selectedSetting, currentSettings);
          break;
      }
    
      if(menuIsOpen)                                     // If the menu is still open
      {
        displayMenu(currentMenu, selectedIndex);                                   // display the menu
      }
    }

    if(right.buttonIsPressed())                          // Pressing the right button moves the cursor to the next item (if there is one)
    {
      if(currentMenu[selectedIndex + 1].functionID != END_MARKER) 
      {
        selectedIndex++;
      }
    
      displayMenu(currentMenu, selectedIndex);
    }
    
    delay(100);
  }


  
  if(hour(now()) == nextSyncHour)                        // If the local clock needs to be synced
  {
    syncTime();                                          // Sync the local time to the NTP server

    calcNextSync();                                      // and calculate when the clock will next need to be synced
  }

  manageOutputs();                                       // Turn on or off the light bar, buzzer or USB ports depending on the time

  delay(100);
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

  display.clearDisplay(); 
  display.display();
    
  WiFi.mode(WIFI_OFF);                              // Turn off the WiFi

}

//---------------------------------------------- C A L C U L A T E   N E X T   S Y N C ----------------------------------------------

void calcNextSync()
{
  nextSyncHour = hour(now()) + currentSettings.hrsBetweenSync;      // Add to the current hour to find when the clock next needs to sync

  while(nextSyncHour >= 24)                       
  {
    nextSyncHour = nextSyncHour - 24;             
  }
}



//--------------------------------------------------- M A N A G E   O U T P U T S ---------------------------------------------------

void manageOutputs()
{
  lightBar.manageOutput(now());    // Turn on the light bar when it is scheduled                    
  

  if(hour(now()) >= currentSettings.buzzerOnTime && hour(now()) < currentSettings.buzzerOffTime)        // Sound the audio alarm when it is scheduled                
  {
    if(buzzerEnabled)                                                   // provided that it is enabled
    {
      tone(buzzerPin, 2000);
      delay(1000);
      noTone(buzzerPin);
      delay(1000);                                      
    }
  }
  else
  {
    noTone(buzzerPin);
  }


  usb1.manageOutput(now());             // Turn on USB port 1 when it is scheduled                     

  usb2.manageOutput(now());             // Turn on USB port 2 when it is scheduled                     

  usb3.manageOutput(now());             // Turn on USB port 3 when it is scheduled                     
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

  display.clearDisplay(); // Clear the Display when the user exits the screen
}



//-------------------------------------------- M A N A G E   S C R E E N   T I M E O U T --------------------------------------------

void manageScreenTimeout()
{
  if(itIsNight())                                                                             // At night (22:00-07:00 by default)
  {
    if (hour(now()) >= screenTimeoutStart && minute(now()) == 0 && second(now()) <= 10)       // When it initially turns night
    {
      screenIsOn = false;                                                                     // set the screen to turn off
    }
    if(minute(now()) == screenTimeoutMin && second(now()) == screenTimeoutSec && screenIsOn)  // if the screen is on and due to timeout
    {     
      screenIsOn = false;                                                                     // set the screen to turn off
    }
    
    while(!screenIsOn)                                                                        // while the screen is set to be off
    {
      display.clearDisplay();                                                                 // make the screen blank                        
      display.display();

      manageOutputs();                                                                        // Turn on or off the light bar, buzzer or USB ports depending on the time

      if(anyButtonIsPressed())                                                                // if any button is pressed
      {
        screenIsOn = true;                                                                    // set the screen to be on
  
        screenTimeoutMin = minute(now()) + 1;                                                 // and set it to timeout again in a minute
        screenTimeoutSec = second(now());
      }

      if(!itIsNight())                                                                        // if it is no longer night
      {
        screenIsOn = true;                                                                    // set the screen to turn on
      }
      
      delay(100);
    }   
  }
  else                                                                                        // In the day                          
  {
    screenIsOn = true;                                                                        // the screen is always on
  }
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
}

//------------------------------------------------------ I T   I S   N I G H T ------------------------------------------------------

bool itIsNight()
{
  return (hour(now()) >= screenTimeoutStart || hour(now()) < screenTimeoutEnd);               // At night (22:00-07:00 by default)
}

//-------------------------------------------- A N Y   B U T T O N   I S   P R E S S E D --------------------------------------------

bool anyButtonIsPressed()
{
  return (left.buttonIsPressed() || middle.buttonIsPressed() || right.buttonIsPressed());
}
