#include "Time.h"

void Time::update(timeStruct newTime) {
  m_actualTime = newTime;

  if (m_actualTime.day != m_oldTime.day && m_onDayChangedListener != 0)
    m_onDayChangedListener(m_actualTime);
  else if (m_actualTime.hours != m_oldTime.hours && m_onHourChangedListener != 0)
    m_onHourChangedListener(m_actualTime);
  else if (m_actualTime.minutes != m_oldTime.minutes && m_onMinuteChangedListener != 0)
    m_onMinuteChangedListener(m_actualTime);
  else if (m_actualTime.seconds != m_oldTime.seconds && m_onSecondChangedListener != 0)
    m_onSecondChangedListener(m_actualTime);

  m_oldTime = m_actualTime;
}

void Time::setOnSecondChangedListener(void(*onSecondChanged)(timeStruct actualTime)) {
  m_onSecondChangedListener = onSecondChanged;
}

void Time::setOnMinuteChangedListener(void(*onMinuteChanged)(timeStruct actualTime)) {
  m_onMinuteChangedListener = onMinuteChanged;
}

void Time::setOnHourChangedListener(void(*onHourChanged)(timeStruct actualTime)) {
  m_onHourChangedListener = onHourChanged;
}

void Time::setOnDayChangedListener(void(*onDayChanged)(timeStruct actualTime)) {
  m_onDayChangedListener = onDayChanged;
}

void Time::clearTimeStruct(timeStruct *structToClear) {
  structToClear->year = 2018;
  structToClear->month = 1;
  structToClear->day = 1;
  structToClear->hours = 0;
  structToClear->minutes = 0;
  structToClear->seconds = 0;
}

Time::timeStruct Time::rtcDateTimeToTimeStruct(const RtcDateTime dateTimeToConvert) {
  timeStruct returnTime;
  
  returnTime.year = dateTimeToConvert.Year();
  returnTime.month = dateTimeToConvert.Month();
  returnTime.day = dateTimeToConvert.Day();
  returnTime.hours = dateTimeToConvert.Hour();
  returnTime.minutes = dateTimeToConvert.Minute();
  returnTime.seconds = dateTimeToConvert.Second();

  return returnTime;
}

bool Time::isLeapYear(uint16_t year) {
  if (year % 4 == 0 || year % 400 == 0)
    return true;
  else
    return false;
}

uint8_t Time::getMaxDay(uint8_t month, uint16_t year) {
  if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
    return 31;
  else if (month == 4 || month == 6 || month == 9 || month == 11)
    return 30;
  else if (month == 2 && isLeapYear(year))
    return 29;
  else if (month == 2 && !isLeapYear(year))
    return 28;
  else
    return 0;
}

String Time::getFormatted24Hrs(int hours, int minutes, int selectedNumber, bool is12Hrs, bool showTwoPoints) {
  String returnString;
  bool isPM = false;

  if (selectedNumber == 1)
    returnString += "[";

  if (is12Hrs) {
    if (hours >= 12)
      isPM = true;

    hours %= 12;

    if (hours == 0)
      hours = 12;
  }

  if (hours < 10)
    returnString += "0" + String(hours);
  else
    returnString += String(hours);

  if (selectedNumber == 1)
    returnString += "]";

  if (showTwoPoints)
    returnString += ":";
  else
    returnString += " ";

  if (selectedNumber == 2)
    returnString += "[";

  if (minutes < 10)
    returnString += "0" + String(minutes);
  else
    returnString += String(minutes);

  if (selectedNumber == 2)
    returnString += "]";

  if (is12Hrs) {
    if (isPM)
      returnString += " PM";
    else
      returnString += " AM";
  }

  return returnString;
}

int Time::getWeekOfTheYear(const timeStruct timeToParse) {
  timeStruct tmFirstJan = timeToParse;
  tmFirstJan.month = 1;
  tmFirstJan.day = 1;
  
  const int dayOfYear = getDayOfTheYear(timeToParse);
  const int dayOfWeek = getDayOfTheWeek(timeToParse);
  const int dayOfWeekFirstJan = getDayOfTheWeek(tmFirstJan);

  int returnedWeekOfYear = ((dayOfYear + 6) / 7);
  if (dayOfWeek < dayOfWeekFirstJan)
    returnedWeekOfYear++;

  return returnedWeekOfYear;
}

int Time::getDayOfTheYear(const timeStruct timeToParse) {
  int returnedDay = 0;
  
  for (int i = 1; i < timeToParse.month; i++)
    returnedDay += getMaxDay(i, timeToParse.year);

  returnedDay += timeToParse.day;

  return returnedDay;
}

Time::WeekDays Time::getDayOfTheWeek(const timeStruct timeToParse) {
  timeStruct tm = timeToParse;
  return (tm.day += tm.month < 3 ? tm.year-- : tm.year - 2, 23 * tm.month / 9 + tm.day + 4 + tm.year / 4 - tm.year / 100 + tm.year / 400) % 7;
}

bool Time::setRTCTime(const timeStruct timeToSet) {
  RtcDS3231<TwoWire> rtc(Wire);
  RtcDateTime dateTimeToSet = RtcDateTime(timeToSet.year, timeToSet.month, timeToSet.day, timeToSet.hours, timeToSet.minutes, timeToSet.seconds);

  rtc.Begin();

  if (rtc.GetIsRunning()) {
    rtc.SetDateTime(dateTimeToSet);
    return true;
  }

  return false;
}

Time::timeStruct Time::getRTCTime() {
  RtcDS3231<TwoWire> rtc(Wire);
  rtc.Begin();

  timeStruct returnTime;
  clearTimeStruct(&returnTime);

  if (rtc.GetIsRunning())
    returnTime = rtcDateTimeToTimeStruct(rtc.GetDateTime());

  return returnTime;
}

String Time::formatNumber(int number) {
  if (number < 10)
    return String('0' + String(number));
  else
    return String(number);
}
