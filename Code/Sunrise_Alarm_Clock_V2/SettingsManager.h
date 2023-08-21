// SettingsManager.h

#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <EEPROM.h>

struct Settings {
    uint8_t lightBarOnTime;
    uint8_t lightBarOffTime;

    uint8_t buzzerOnTime;
    uint8_t buzzerOffTime;

    uint8_t usb1OnTime;
    uint8_t usb1OffTime;

    uint8_t usb2OnTime;
    uint8_t usb2OffTime;

    uint8_t usb3OnTime;
    uint8_t usb3OffTime;

    int8_t utcOffset;
    uint8_t hrsBetweenSync;
};

//extern Settings currentSettings;

//void adjustSetting(FunctionID functionID);
void saveSettings(const Settings &settings);
void loadSettings(Settings &currentSettings);

#endif // SETTINGS_MANAGER_H
