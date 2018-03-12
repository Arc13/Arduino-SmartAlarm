#include "Settings.h"

Settings::settingBlock1 Settings::getSettingBlock1() {
  settingBlock1 settingBlock;
  
  EEPROM.get(0x1, settingBlock);
  return settingBlock;
}

void Settings::setSettingBlock1(const settingBlock1 blockToSet) {
  EEPROM.update(0x1, blockToSet.settingBlockByte[0]);
}

