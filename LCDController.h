#ifndef LCDCONTROLLER_H
#define LCDCONTROLLER_H

#include "Arduino.h"
#include "AlarmUtilities.h"
#include "Time.h"
#include <LiquidCrystal.h>

class LCDController {
  public:
    enum DisplayMode {
      MODE_IDLE = 1,

      MODE_MAINMENU,
      MODE_SETTINGS,

      MODE_LISTALARM,
      MODE_DELETEALARM,
      
      MODE_ALARMCREATE1,
      MODE_ALARMCREATE2,
      MODE_ALARMCREATE3,
      MODE_ALARMCREATE4,
      MODE_ALARMCREATE5,

      MODE_SETCLOCK1,
      MODE_SETCLOCK2,
      MODE_SETCLOCK3,

      MODE_NOTIFICATION,
    };

    enum Actions {
      ACTION_NONE = 0,
      
      ACTION_UP = 1,
      ACTION_DOWN,
      ACTION_LEFT,
      ACTION_RIGHT,

      ACTION_MAIN,
    };
  
    LCDController(int rs, int enable, int d4, int d5, int d6, int d7, int rows, int columns);

    void setMode(DisplayMode newMode);
    DisplayMode getMode();
    
    void setPosition(int position);
    int getPosition();

    void setSecondaryData(int secondaryData);
    int getSecondaryData();

    void setIdleMessage(String clock, String temperature, String date);
    void sendNotification(String message, unsigned long notificationDuration = 3000);
    
    void updateScreen();
    void update();
    void actionHandler(Actions action);

  private:
    String getStringWithSelector(int pos, String text);
    void checkPositionAndMode();
    void updateIdleScreen(String clock, String temperature, String date, bool forceUpdate = false);
    
    LiquidCrystal m_lcd;
    
    DisplayMode m_actualMode;
    String m_clock;
    String m_temperature;
    String m_date;

    AlarmUtils::alarm m_alarmToCreate;
    Time::timeStruct m_clockToSet;
    
    int m_position;
    int m_secondaryData;

    unsigned long m_startTimeNotification;
    unsigned long m_endTimeNotification;
    unsigned long m_oldTimePassedChar;
    String m_notification;
    DisplayMode m_modeBeforeNotification;

    unsigned long m_lastEvent;
};

#endif
