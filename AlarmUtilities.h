#ifndef ALARMUTILS_H
#define ALARMUTILS_H

#include "Arduino.h"
#include "Time.h"

class AlarmUtils {
  public:
    // Union containing the alarm save structure in EEPROM accessible by a struct and a byte
    typedef union {
      struct alStruct {
        byte hours     : 5;
        // Une heure (comprise entre 0 et 23, stocké donc sur 5 bits car cela offre 32 nombres, 4 bits en offrent 16)
        byte evenWeeks : 1; // Semaines paires (1 bit actif (état 0) ou inactif (état 1))
        byte oddWeeks  : 1; // Semaines impaires (1 bit aussi)
        byte reserved1 : 1; // 1 bit inutilisé
        byte minutes   : 6;
        // Une minute (comprise entre 0 et 59, stocké donc sur 6 bits car cela offre 64 nombres, 5 bits en offrent 32)
        byte reserved2 : 2; // 2 bits inutilisés
        byte dayOfWeek : 7; // Jours de la semaine (Stocké sur 7 bits, un bit par jour)
        byte isEnabled : 1; // Stocke l'état de l'alarme (Activée ou désactivée, 1 bit aussi)
      } alStruct;
      
      byte alByte[sizeof(alStruct)];
    } alarm;

    static void readAlarmFromEEPROM(int index, alarm *alarmToRead);
    static void writeAlarmToEEPROM(int index, const alarm alarmToWrite);

    static void addAlarm(alarm alarmToWrite);
    static void removeAlarm(int index);
    
    static void clearAlarm(alarm *alarmToClear);
    static void defaultAlarm(alarm *alarmToSet);
    
    static int getAlarmCount();

    static bool isDayWeekEnabled(int dayOfWeek, alarm alarmToRead);

    static void setAlarmEnabled(int index, bool enable);
    static bool isAlarmEnabled(int index);

    static bool isAlarmRunning(Time::timeStruct actualTime);

  private:
    static int getAddressFromIndex(int index);
    
};

#endif
