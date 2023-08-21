// SettingsManager.cpp

#include "SettingsManager.h"

//---------------------------------------------------- S A V E   S E T T I N G S ----------------------------------------------------

void saveSettings(const Settings &settings) 
{
  uint8_t address = 0;

  EEPROM.put(address, settings); // Write the entire struct at once
  EEPROM.commit();
}

//---------------------------------------------------- L O A D   S E T T I N G S ----------------------------------------------------

void loadSettings(Settings &settings)
{
  uint8_t address = 0;

  EEPROM.get(address, settings);
}