#include <TimeLib.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

const long utcOffsetInSeconds = 3600;       // BST = (3600 secs) ahead of UTC and GMT= (0 secs) ahead of UTC

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int nextSyncHour = 0;                       // Variable to store the next hour when the clock needs to be synced
int hrsBetweenSync = 12;                    // How often the clock needs to be synced (hours) 

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//------------------------------------------------------------ S E T U P ------------------------------------------------------------

void setup(){
  Serial.begin(115200);
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);   // Setup the OLED Display
  display.clearDisplay();                                
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  syncTime();                                            // Sync the local time to the NTP server
  calcNextSync();                                        // Calculate when the clock will next need to be synced
}

//------------------------------------------------------------- M A I N -------------------------------------------------------------

void loop() 
{
  displayTime(now());                                    // Display the time
  
  if (hour(now()) == nextSyncHour)                       // If the local clock needs to be synced
  {
    syncTime();                                          // Sync the local time to the NTP server

    calcNextSync();                                      // and calculate when the clock will next need to be synced
  }
}

//-------------------------------------------------------- S Y N C   T I M E --------------------------------------------------------

void syncTime()
{
  WiFi.mode(WIFI_STA);                            // Turn the WiFi on  
  WiFi.begin(ssid, password);                    

  display.clearDisplay();                         // Clear the Display
  display.setCursor(0,0);
  display.println("Connecting");                  // Show that the device is connecting to the WiFi
  display.display();

  display.setCursor(0,16);


  while ( WiFi.status() != WL_CONNECTED ) {       // Until the WiFi Connects
    display.print(".");                           // Put dots on the screen to show it hasn't frozen
    display.display();
    delay ( 500 );
  }

  display.clearDisplay();                          
  display.setCursor(0,0);
  display.println("Connected");                   // Show that the alarm has connected to the WiFi
  display.display();
  delay(1000);

  display.clearDisplay(); 
  display.display();

  timeClient.begin();                             // Connect to the NTP Server
  timeClient.update();                            // Get the time from the NTP Server

  setTime(timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds(), timeClient.getDay(),0,0); // Set the local time

  WiFi.mode(WIFI_OFF);                            // Turn off the WiFi

}

//---------------------------------------------- C A L C U L A T E   N E X T   S Y N C ----------------------------------------------

void calcNextSync()
{
  nextSyncHour = hour(now()) + hrsBetweenSync;    // Add to the current hour to find when the clock next needs to sync

  while(nextSyncHour >= 24)                       
  {
    nextSyncHour = nextSyncHour - 24;             
  }
}

//----------------------------------------------------- D I S P L A Y   T I M E -----------------------------------------------------

void displayTime(time_t t)
{
  display.clearDisplay();  
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
