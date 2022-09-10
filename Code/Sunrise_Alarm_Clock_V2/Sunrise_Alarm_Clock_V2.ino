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

#include <EEPROM.h>

#include "Button.h"
#include "Menu.h"
#include "MenuItem.h"
#include "Icons.h"

//----------------------------------------------------------- P I N O U T -----------------------------------------------------------

#define buttonsPin A0
#define lightBarPin D0
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

bool screenIsOn = true;                                                     // turns the display on or off
uint8_t screenTimeoutStart = 22;                                            // The screen will be able to timeout from 22:00
uint8_t screenTimeoutEnd = 7;                                               // to 07:00
uint8_t screenTimeoutSec = 0;                                               // variable to store the second when the screen will time out 
uint8_t screenTimeoutMin = 0;                                               // variable to store the minute when the screen will time out 


//------------------------------------------------- W I F I   C R E D E N T I A L S -------------------------------------------------  

const char *ssid     = "YOUR_WIFI_SSID";        // REPLACE WITH YOUR WIFI SSID
const char *password = "YOUR_WIFI_PASSWORD";    // REPLACE WITH YOUR WIFI PASSWORD


//-------------------------------------------- T I M E   &   D A T E   V A R I A B L E S --------------------------------------------

long utcOffsetInSeconds = 3600;             // BST = (3600 secs) ahead of UTC and GMT= (0 secs) ahead of UTC

char daysOfTheWeek[7][12] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

uint8_t nextSyncHour = 0;                   // Variable to store the next hour when the clock needs to be synced
uint8_t hrsBetweenSync = 12;                // How often the clock needs to be synced (hours) 

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

uint8_t ntpConnectionTimeout = 30;          // How many seconds before giving up connecting to the NTP server   


//----------------------------------------------- S E T T I N G S   V A R I A B L E S -----------------------------------------------

Menu mainMenu;                                            // Instantiates a menu called mainMenu

uint8_t lightBarOnTime = 0;
uint8_t lightBarOffTime = 0;
bool lightBarState = 0;

uint8_t buzzerOnTime = 0;
uint8_t buzzerOffTime = 0;
bool buzzerEnabled = true;

uint8_t usb1OnTime = 0;
uint8_t usb1OffTime = 0;

uint8_t usb2OnTime = 0;
uint8_t usb2OffTime = 0;

uint8_t usb3OnTime = 0;
uint8_t usb3OffTime = 0;

uint8_t address = 0;

//------------------------------------------------------------ S E T U P ------------------------------------------------------------

void setup(){
  Serial.begin(115200);

  EEPROM.begin(512);

  loadSettings();                                        // Load settings from EEPROM

  pinMode(lightBarPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(usb1Pin, OUTPUT);
  pinMode(usb2Pin, OUTPUT);
  pinMode(usb3Pin, OUTPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);   // Setup the OLED Display
  display.clearDisplay();                                
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  
  syncTime();                                            // Sync the local time to the NTP server
  calcNextSync();                                        // Calculate when the clock will next need to be synced

  buildMenu(mainMenu);                                   // build the mainMenu

  screenTimeoutMin = minute(now()) + 1;                  // set the screen to timeout 1 minute after turning on
  screenTimeoutSec = second(now());                      // (the screen will only timeout at night)
}


//------------------------------------------------------------- M A I N -------------------------------------------------------------

void loop() 
{

  manageScreenTimeout();                                 // Manage the screen timeout depending on the time of day

  if(screenIsOn)                                         // If the screen is on
  {
    displayTime(now());                                  // display the time
    displayIcons();                                      // and the icons
  }

  
  if(left.buttonIsPressed())                             // Pressing the left button toggles the light bar on or off
  {
    if(lightBarState == 0) 
    {
      Serial.println("Turn on Light Bar");
      digitalWrite(lightBarPin, HIGH);
      lightBarState = 1;
    }
    else
    {
      Serial.println("Turn off Light Bar");
      digitalWrite(lightBarPin, LOW);
      lightBarState = 0;
    }

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
    display.clearDisplay();                                
    display.display();
    display.setCursor(0,24);
    display.println(" Main Menu ");
    display.drawRect(0, 0, 128, 64, WHITE);    
    display.display();      
    delay(1000);                       
    mainMenu.open(); 
  }



  while(mainMenu.isOpen)                                 // When the menu is open
  {
    displayIcons();
    
    if(left.buttonIsPressed())                           // Pressing the left button moves the cursor to the previous item
    {
      mainMenu.moveToPreviousItem();
      mainMenu.displayMenu();
    }
    
    if(middle.buttonIsPressed())                         // Pressing the middle button selects the item highlighted by the cursor
    {
      int selectedSetting = mainMenu.selectItem();

      if(selectedSetting > 1 && selectedSetting != NULL) // If the selected item has a valid functionID
      {
        adjustSetting(selectedSetting);                  // advance to a screen where that setting can be adjusted
      }
    
      if(mainMenu.isOpen)                                // If the menu is still open
      {
        mainMenu.displayMenu();                          // display the menu
      }
      else                                               // if the menu has been closed
      {
        screenTimeoutMin = minute(now()) + 1;            // set the screen to timeout in a minute
        screenTimeoutSec = second(now());      
      }
    }

    if(right.buttonIsPressed())                          // Pressing the right button moves the cursor to the next item  
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

  

  if(hour(now()) >= lightBarOnTime && hour(now()) < lightBarOffTime)    // Turn on the light bar when it is scheduled                    
  {
    digitalWrite(lightBarPin, HIGH); 
    lightBarState = 1;                                     
  }
  if(hour(now()) == lightBarOffTime && minute(now()) == 0)                
  {
    digitalWrite(lightBarPin, LOW);
    lightBarState = 0;
  }


  if(hour(now()) >= buzzerOnTime && hour(now()) < buzzerOffTime)        // Sound the audio alarm when it is scheduled                
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


  if(hour(now()) >= usb1OnTime && hour(now()) < usb1OffTime)            // Turn on USB port 1 when it is scheduled                     
  {
    digitalWrite(usb1Pin, HIGH);                                      
  }
  else
  {
    digitalWrite(usb1Pin, LOW);
  }


  if(hour(now()) >= usb2OnTime && hour(now()) < usb2OffTime)            // Turn on USB port 2 when it is scheduled                     
  {
    digitalWrite(usb2Pin, HIGH);                                      
  }
  else
  {
    digitalWrite(usb2Pin, LOW);
  }


  if(hour(now()) >= usb3OnTime && hour(now()) < usb3OffTime)            // Turn on USB port 3 when it is scheduled                     
  {
    digitalWrite(usb3Pin, HIGH);                                      
  }
  else
  {
    digitalWrite(usb3Pin, LOW);
  }

  delay(100);
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
  nextSyncHour = hour(now()) + hrsBetweenSync;      // Add to the current hour to find when the clock next needs to sync

  while(nextSyncHour >= 24)                       
  {
    nextSyncHour = nextSyncHour - 24;             
  }
}

//----------------------------------------------------- D I S P L A Y   T I M E -----------------------------------------------------

void displayTime(time_t t)
{ 
  display.setTextSize(2); 
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,5);
  display.print("           ");

  switch(day(t))                                   // Centre the text
  {
    case 0: display.setCursor(28,5); break;
    case 1: display.setCursor(22,5); break;
    case 2: display.setCursor(10,5); break;
    case 3: display.setCursor(16,5); break;
    case 4: display.setCursor(28,5); break;
    case 5: display.setCursor(16,5); break;
    case 6: display.setCursor(28,5); break;
  }

  display.println(daysOfTheWeek[day(t)]);           

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

//---------------------------------------------------- D I S P L A Y   I C O N S ----------------------------------------------------
void displayIcons()
{
  if(mainMenu.isOpen)
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

//--------------------------------------------------- A D J U S T   S E T T I N G ---------------------------------------------------

void adjustSetting(int functionID)
{
  delay(500);
  
  display.setTextSize(2); 
  display.setTextColor(WHITE, BLACK);

  display.drawBitmap(2, 48, downArrowIcon, 16, 16, WHITE);
  display.drawBitmap(110, 48, upArrowIcon, 16, 16, WHITE);
  
  
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

        if(left.buttonIsPressed() && lightBarOnTime > 0)        // press the left button to decrement the on time (down to 00:00)
        {
          lightBarOnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && lightBarOnTime < 23)      // press the right button to increment the on time (up to 23:00)
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

        if(left.buttonIsPressed() && lightBarOffTime > 0)       // press the left button to decrement the off time (down to 00:00)
        {
          lightBarOffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && lightBarOffTime < 23)     // press the right button to increment the off time (up to 23:00)
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

        if(left.buttonIsPressed() && buzzerOnTime > 0)          // press the left button to decrement the on time (down to 00:00)
        {
          buzzerOnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && buzzerOnTime < 23)        // press the right button to increment the on time (up to 23:00)
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

        if(left.buttonIsPressed() && buzzerOffTime > 0)         // press the left button to decrement the off time (down to 00:00)
        {
          buzzerOffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && buzzerOffTime < 23)       // press the right button to increment the off time (up to 23:00)
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

        if(left.buttonIsPressed() && usb1OnTime > 0)            // press the left button to decrement the on time (down to 00:00)
        {
          usb1OnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb1OnTime < 23)          // press the right button to increment the on time (up to 23:00)
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

        if(left.buttonIsPressed() && usb1OffTime > 0)           // press the left button to decrement the off time (down to 00:00)
        {
          usb1OffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb1OffTime < 23)         // press the right button to increment the off time (up to 23:00)
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

        if(left.buttonIsPressed() && usb2OnTime > 0)            // press the left button to decrement the on time (down to 00:00)
        {
          usb2OnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb2OnTime < 23)          // press the right button to increment the on time (up to 23:00)
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

        if(left.buttonIsPressed() && usb2OffTime > 0)           // press the left button to decrement the off time (down to 00:00)
        {
          usb2OffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb2OffTime < 23)         // press the right button to increment the off time (up to 23:00)
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

        if(left.buttonIsPressed() && usb3OnTime > 0)            // press the left button to decrement the on time (down to 00:00)
        {
          usb3OnTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb3OnTime < 23)          // press the right button to increment the on time (up to 23:00)
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

        if(left.buttonIsPressed() && usb3OffTime > 0)           // press the left button to decrement the off time (down to 00:00)
        {
          usb3OffTime--;
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && usb3OffTime < 23)         // press the right button to increment the off time (up to 23:00)
        {
          usb3OffTime++; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case TIME_OFFSET:
        display.print(utcOffsetInSeconds/3600);                 // print the number of hours offset from UTC
        display.print(" Hours");

        if(left.buttonIsPressed() && hrsBetweenSync > -12)      // press the left button to decrement the offset time (down to -12)
        {
          utcOffsetInSeconds = utcOffsetInSeconds - 3600; 
          display.setCursor(0,16);
          display.print("           ");
        } 
        if(right.buttonIsPressed() && hrsBetweenSync < 12)      // press the right button to increment the offset time (up to +12)
        {
          utcOffsetInSeconds = utcOffsetInSeconds + 3600; 
          display.setCursor(0,16);
          display.print("           ");
        }
        break;
        
      case HOURS_BETWEEN_SYNCS:      
        display.print(hrsBetweenSync);                          // print the number of hours between syncs
        display.print(" Hours");

        if(left.buttonIsPressed() && hrsBetweenSync > 1)        // press the left button to decrement the number of hours (down to 1) 
        {
          hrsBetweenSync--; 
          display.setCursor(0,16);
          display.print("           ");        
        } 
        if(right.buttonIsPressed() && hrsBetweenSync < 24)      // press the right button to increment the number of hours (up to 24)
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

      case SAVE:
        display.print("Saving...");
        display.display();
        
        saveSettings();                                         // save the current settings to EEPROM
        
        delay(1000);
        display.clearDisplay();
        display.setCursor(0,16);
        display.print("Saved");
        display.display();
        delay(1000);
        
        display.clearDisplay();
        display.display();
        return;
    }
    
    display.display();
    delay(100);
  }

  display.clearDisplay();                                       // Clear the Display when the user exits the screen
}

//---------------------------------------------------- S A V E   S E T T I N G S ----------------------------------------------------

void saveSettings()
{
  address = 0;

  EEPROM.put(address, lightBarOnTime);

  address = address + sizeof(lightBarOnTime);
  EEPROM.put(address, lightBarOffTime);

  address = address + sizeof(lightBarOffTime);
  EEPROM.put(address, buzzerOnTime);

  address = address + sizeof(buzzerOnTime);
  EEPROM.put(address, buzzerOffTime);

  address = address + sizeof(buzzerOffTime);
  EEPROM.put(address, usb1OnTime);

  address = address + sizeof(usb1OnTime);
  EEPROM.put(address, usb1OffTime);

  address = address + sizeof(usb1OffTime);
  EEPROM.put(address, usb2OnTime);

  address = address + sizeof(usb2OnTime);
  EEPROM.put(address, usb2OffTime);

  address = address + sizeof(usb2OffTime);
  EEPROM.put(address, usb3OnTime);

  address = address + sizeof(usb3OnTime);
  EEPROM.put(address, usb3OffTime);

  address = address + sizeof(usb3OffTime);
  EEPROM.put(address, utcOffsetInSeconds);

  address = address + sizeof(utcOffsetInSeconds);
  EEPROM.put(address, hrsBetweenSync);

  EEPROM.commit();
}

//---------------------------------------------------- L O A D   S E T T I N G S ----------------------------------------------------

void loadSettings()
{
  address = 0;

  EEPROM.get(address, lightBarOnTime);

  address = address + sizeof(lightBarOnTime);
  EEPROM.get(address, lightBarOffTime);

  address = address + sizeof(lightBarOffTime);
  EEPROM.get(address, buzzerOnTime);

  address = address + sizeof(buzzerOnTime);
  EEPROM.get(address, buzzerOffTime);

  address = address + sizeof(buzzerOffTime);
  EEPROM.get(address, usb1OnTime);

  address = address + sizeof(usb1OnTime);
  EEPROM.get(address, usb1OffTime);

  address = address + sizeof(usb1OffTime);
  EEPROM.get(address, usb2OnTime);

  address = address + sizeof(usb2OnTime);
  EEPROM.get(address, usb2OffTime);

  address = address + sizeof(usb2OffTime);
  EEPROM.get(address, usb3OnTime);

  address = address + sizeof(usb3OnTime);
  EEPROM.get(address, usb3OffTime);

  address = address + sizeof(usb3OffTime);
  EEPROM.get(address, utcOffsetInSeconds);

  address = address + sizeof(utcOffsetInSeconds);
  EEPROM.get(address, hrsBetweenSync);
}

//-------------------------------------------- M A N A G E   S C R E E N   T I M E O U T --------------------------------------------

void manageScreenTimeout()
{
  if(hour(now()) >= screenTimeoutStart || hour(now()) < screenTimeoutEnd)                     // At night (22:00-07:00 by default)
  {
    if(minute(now()) == screenTimeoutMin && second(now()) == screenTimeoutSec && screenIsOn)  // if the screen is on and due to timeout
    {     
      screenIsOn = false;                                                                     // set the screen to turn off
    }
    while(!screenIsOn)                                                                        // while the screen is set to be off
    {
      display.clearDisplay();                                                                 // make the screen blank                        
      display.display();
      
      if(left.buttonIsPressed() || middle.buttonIsPressed() || right.buttonIsPressed())       // and wait for any of the buttons to be pressed
      {
        screenIsOn = true;                                                                    // when a button is pressed, set the screen to be on
  
        screenTimeoutMin = minute(now()) + 1;                                                 // and set it to timeout again in a minute
        screenTimeoutSec = second(now());
      }
      delay(100);
    }
  }
  else                                                                                        // In the day                          
  {
    screenIsOn = true;                                                                        // the screen is always on
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

  currentItem = currentItem->previousMenu;

  currentItem->addNewItem("Save Settings", SAVE);

  free(currentItem);                                                    // Delete the currentItem pointer (no longer needed).
}
