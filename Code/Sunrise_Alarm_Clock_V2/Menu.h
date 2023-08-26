
#ifndef MENU_H
#define MENU_H

/*
  The following code will build this menu:

  - Main Menu
    ├── Alarm
    │   ├── Light Bar
    │   │   ├── On Time
    │   │   └── Off Time
    │   └── Buzzer
    │       ├── On Time
    │       └── Off Time
    ├── USB Ports
    │   ├── USB 1
    │   │   ├── On Time
    │   │   └── Off Time
    │   ├── USB 2
    │   │   ├── On Time
    │   │   └── Off Time
    │   └── USB 3
    │       ├── On Time
    │       └── Off Time
    ├── Time
    │   ├── Time Offset (hrs)
    │   ├── Hours Between Syncs
    │   └── Sync Time Now
    └── Save

*/


MenuItem lightBarMenu[] = {
  {"Back", BACK, nullptr},
  {"On Time", LIGHT_BAR_ON_TIME, nullptr},
  {"Off Time", LIGHT_BAR_OFF_TIME, nullptr},
  {"", END_MARKER, nullptr}
};

MenuItem buzzerMenu[] = {
  {"Back", BACK, nullptr},
  {"On Time", BUZZER_ON_TIME, nullptr},
  {"Off Time", BUZZER_OFF_TIME, nullptr},
  {"", END_MARKER, nullptr}
};

MenuItem alarmMenu[] = {
  {"Back", BACK, nullptr},
  {"Light Bar", SUBMENU, lightBarMenu},
  {"Buzzer", SUBMENU, buzzerMenu},
  {"", END_MARKER, nullptr}
};



MenuItem usb1Menu[] = {
  {"Back", BACK, nullptr},
  {"On Time", USB_1_ON_TIME, nullptr},
  {"Off Time", USB_1_OFF_TIME, nullptr},
  {"", END_MARKER, nullptr}
};

MenuItem usb2Menu[] = {
  {"Back", BACK, nullptr},
  {"On Time", USB_2_ON_TIME, nullptr},
  {"Off Time", USB_2_OFF_TIME, nullptr},
  {"", END_MARKER, nullptr}
};

MenuItem usb3Menu[] = {
  {"Back", BACK, nullptr},
  {"On Time", USB_3_ON_TIME, nullptr},
  {"Off Time", USB_3_OFF_TIME, nullptr},
  {"", END_MARKER, nullptr}
};

MenuItem usbPortsMenu[] = {
  {"Back", BACK, nullptr},
  {"USB 1", SUBMENU, usb1Menu},
  {"USB 2", SUBMENU, usb2Menu},
  {"USB 3", SUBMENU, usb3Menu},
  {"", END_MARKER, nullptr}   
};



MenuItem timeMenu[] = {
  {"Back", BACK, nullptr},
  {"Time Offset (hrs)", TIME_OFFSET, nullptr},
  {"Hours Between Syncs", HOURS_BETWEEN_SYNCS, nullptr},
  {"Sync Time Now", SYNC_TIME_NOW, nullptr},
  {"", END_MARKER, nullptr}
};



MenuItem mainMenu[] = {
  {"Exit", EXIT, nullptr},
  {"Alarm", SUBMENU, alarmMenu},
  {"USB Ports", SUBMENU, usbPortsMenu},
  {"Time", SUBMENU, timeMenu},
  {"Save", SAVE, nullptr},
  {"", END_MARKER, nullptr}
};

#endif // MENU_H