#ifndef SETTINGS_H
#define SETTINGS_H

#include "Arduino.h"
#include <EEPROM.h>

class Settings {
  public:
    enum SettingType {
      SETTING_12HRS = 1,
      SETTING_DEGFAHRENHEIT,
    };
    
    typedef union {
      struct settingBlockStruct {
        byte is12HrsFormat      : 1;
        byte isDegreeFahrenheit : 1;
        byte reserved           : 6;
      } settingBlockStruct;

      byte settingBlockByte[sizeof(settingBlockStruct)];
    } settingBlock1;

    static settingBlock1 getSettingBlock1();
    static void setSettingBlock1(const settingBlock1 blockToSet);
};

#endif
