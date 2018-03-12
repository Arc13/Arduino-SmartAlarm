#ifndef TIME_H
#define TIME_H

#include "Arduino.h"
#include <RtcDS3231.h>
#include <Wire.h>

class Time {
  public:
    typedef struct timeStruct {
      uint16_t year;
      uint8_t  month;
      uint8_t  day;
      uint8_t  hours;
      uint8_t  minutes;
      uint8_t  seconds;
    } timeStruct;

    enum WeekDays {      
      WD_SUNDAY = 0,
      WD_MONDAY = 1,
      WD_TUESDAY,
      WD_WEDNESDAY,
      WD_THURSDAY,
      WD_FRIDAY,
      WD_SATURDAY,
    };

    void update(timeStruct newTime);

    void setOnSecondChangedListener(void(*onSecondChanged)(timeStruct actualTime));
    void setOnMinuteChangedListener(void(*onMinuteChanged)(timeStruct actualTime));
    void setOnHourChangedListener(void(*onHourChanged)(timeStruct actualTime));
    void setOnDayChangedListener(void(*onDayChanged)(timeStruct actualTime));

    static void clearTimeStruct(timeStruct *structToClear);
    static timeStruct rtcDateTimeToTimeStruct(const RtcDateTime dateTimeToConvert);

    static String getFormatted24Hrs(int hours, int minutes, int selectedNumber, bool is12Hrs, bool showTwoPoints = true);

    static int getWeekOfTheYear(const timeStruct timeToParse);
    static int getDayOfTheYear(const timeStruct timeToParse);
    static WeekDays getDayOfTheWeek(const timeStruct timeToParse);
    static bool isLeapYear(uint16_t year);
    static uint8_t getMaxDay(uint8_t month, uint16_t year);

    static bool setRTCTime(const timeStruct timeToSet);
    static timeStruct getRTCTime();
    
    static String formatNumber(int number);

  private:
    void (*m_onSecondChangedListener)(timeStruct actualTime);
    void (*m_onMinuteChangedListener)(timeStruct actualTime);
    void (*m_onHourChangedListener)(timeStruct actualTime);
    void (*m_onDayChangedListener)(timeStruct actualTime);

    timeStruct m_actualTime;
    timeStruct m_oldTime;

    static bool m_isRTCAvailable;
};

#endif
