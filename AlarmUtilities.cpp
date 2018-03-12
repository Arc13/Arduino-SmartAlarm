#include "AlarmUtilities.h"
#include <EEPROM.h>

// Converts an alarm index into an EEPROM address
int AlarmUtils::getAddressFromIndex(int index) {
  // The address 0 is reserved to contain the number of alarms
  // The address 1 to 9 are reserved for future usage
  // The address 10 to 775 are reserved for the alarms
  
  return 10 + (sizeof(alarm::alStruct) * (index - 1));
}

// Fill an alarm variable with zeroes before any usage
void AlarmUtils::clearAlarm(alarm *alarmToClear) {
  for (int i = 0; i < sizeof(alarm::alStruct); i++) {
    alarmToClear->alByte[i] = 0;
  }
}

// Write an alarm var into the EEPROM
void AlarmUtils::writeAlarmToEEPROM(int index, const alarm alarmToWrite) {
  // Get the address from the index
  int address = getAddressFromIndex(index);

  // Put the content of the struct into the EEPROM
  for (int i = 0; i < sizeof(alarm::alStruct); i++) {
    EEPROM.update(address + i, alarmToWrite.alByte[i]);
  }
}

// Read an alarm from the EEPROM and copy it in an alarm var
void AlarmUtils::readAlarmFromEEPROM(int index, alarm *alarmToRead) {
  // Get the address from the index
  int address = getAddressFromIndex(index);

  // Get the content of the alarm block in the EEPROM
  alarm alarmHolder;
  EEPROM.get(address, alarmHolder);
  memcpy(alarmToRead, &alarmHolder, sizeof(alarm::alStruct));
}

// Automatically add an alarm
void AlarmUtils::addAlarm(alarm alarmToWrite) {
  // Increment the alarm counter
  int numberOfAlarms = EEPROM.read(0);
  numberOfAlarms++;
  EEPROM.update(0, numberOfAlarms);

  // Write the alarm to the last block
  writeAlarmToEEPROM(numberOfAlarms, alarmToWrite);
}

// Automatically remove an alarm
void AlarmUtils::removeAlarm(int index) {
  // Decrement the alarm counter
  int numberOfAlarms = EEPROM.read(0);
  numberOfAlarms--;
  EEPROM.update(0, numberOfAlarms);

  // Clear the alarm block
  int address = getAddressFromIndex(index);
  alarm blankAlarm;
  clearAlarm(&blankAlarm);
  writeAlarmToEEPROM(index, blankAlarm);

  // Shift the EEPROM memory
  for (int i = getAddressFromIndex(index); i < getAddressFromIndex(numberOfAlarms + 1); i += 3) {
    int fromAddress = i + sizeof(alarm::alStruct);
    int toAddress = i;
    
    for (int j = toAddress; j < fromAddress; j++) {
      EEPROM.update(j, EEPROM.read(j + 3));
    }
  }
}

int AlarmUtils::getAlarmCount() {
  return EEPROM.read(0);
}

bool AlarmUtils::isDayWeekEnabled(int dayOfWeek, alarm alarmToRead) {
  if (dayOfWeek == 0)
    return bitRead(alarmToRead.alStruct.dayOfWeek, 6);
  else
    return bitRead(alarmToRead.alStruct.dayOfWeek, dayOfWeek - 1);
}

// Clear an alarm and set it to default values
void AlarmUtils::defaultAlarm(alarm *alarmToSet) {
  clearAlarm(alarmToSet);

  alarmToSet->alStruct.dayOfWeek = B1111111;
  alarmToSet->alStruct.oddWeeks = B1;
  alarmToSet->alStruct.evenWeeks = B1;
  alarmToSet->alStruct.isEnabled = B1;
}

void AlarmUtils::setAlarmEnabled(int index, bool enable) {
  alarm alarmToModify;

  readAlarmFromEEPROM(index, &alarmToModify);
  alarmToModify.alStruct.isEnabled = enable;
  writeAlarmToEEPROM(index, alarmToModify);
}

bool AlarmUtils::isAlarmEnabled(int index) {
  alarm alarmToRead;

  readAlarmFromEEPROM(index, &alarmToRead);
  return alarmToRead.alStruct.isEnabled;
}

bool AlarmUtils::isAlarmRunning(Time::timeStruct actualTime) {
  for (int i = 0; i < getAlarmCount(); i++) {
    alarm currentAlarm;
    clearAlarm(&currentAlarm);
    
    readAlarmFromEEPROM(i + 1, &currentAlarm);

    if (currentAlarm.alStruct.isEnabled) {
      const bool isOddWeek = Time::getWeekOfTheYear(actualTime) % 2;

      if (actualTime.hours == currentAlarm.alStruct.hours && actualTime.minutes == currentAlarm.alStruct.minutes) {
        if ((isOddWeek && currentAlarm.alStruct.oddWeeks) || (!isOddWeek && currentAlarm.alStruct.evenWeeks)) {
          if (isDayWeekEnabled(Time::getDayOfTheWeek(actualTime), currentAlarm)) {
            return true;
          }
        }
      }
    }
  }

  return false;
}

