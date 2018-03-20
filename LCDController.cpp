#include "CommandProcessor.h"
#include "LCDController.h"
#include "LCDCustomChars.h"
#include "Settings.h"

LCDController::LCDController(int rs, int enable, int d4, int d5, int d6, int d7, int columns, int rows)
  : m_lcd(rs, enable, d4, d5, d6, d7)
  , m_actualMode(MODE_IDLE)
  , m_position(1)
  , m_secondaryData(1)
  , m_lastEvent(millis())
{
  m_lcd.begin(columns, rows);

  AlarmUtils::defaultAlarm(&m_alarmToCreate);
  m_position = 1;

  updateScreen();

  m_lcd.createChar(CHAR_DEGREECELSIUS, char_degreeCelsius);
  m_lcd.createChar(CHAR_DEGREEFAHRENHEIT, char_degreeFahrenheit);
}

void LCDController::actionHandler(LCDController::Actions action) {
  if (action == LCDController::ACTION_NONE)
    return;

  m_lastEvent = millis();

  if (m_actualMode == MODE_IDLE) {
    switch (action) {
      case ACTION_DOWN:
        setMode(MODE_MAINMENU);

        break;
      case ACTION_RIGHT:
        if (AlarmUtils::getAlarmCount() > 0)
          setMode(MODE_LISTALARM);

        break;
      case ACTION_MAIN:
        if (m_position == 1)
          m_position = 2;
        else
          m_position = 1;

        updateScreen();
        break;
      default:
        break;
    }
  } else if (m_actualMode >= MODE_ALARMCREATE1 && m_actualMode <= MODE_ALARMCREATE5) {
    switch (action) {
      case ACTION_LEFT:
        if (m_position > 1) {
          m_position--;

          checkPositionAndMode();
        }

        break;
      case ACTION_RIGHT:
        if (m_position < 13) {
          m_position++;

          checkPositionAndMode();
        }

        break;
      case ACTION_UP:
        if (m_position == 1) {
          m_alarmToCreate.alStruct.hours++;

          if (m_alarmToCreate.alStruct.hours > 23)
            m_alarmToCreate.alStruct.hours = 0;
        } else if (m_position == 2) {
          m_alarmToCreate.alStruct.minutes++;

          if (m_alarmToCreate.alStruct.minutes > 59)
            m_alarmToCreate.alStruct.minutes = 0;
        }

        break;
      case ACTION_DOWN:
        if (m_position == 1) {
          m_alarmToCreate.alStruct.hours--;

          if (m_alarmToCreate.alStruct.hours > 23)
            m_alarmToCreate.alStruct.hours = 23;
        } else if (m_position == 2) {
          m_alarmToCreate.alStruct.minutes--;

          if (m_alarmToCreate.alStruct.minutes > 59)
            m_alarmToCreate.alStruct.minutes = 59;
        }

        break;
      case ACTION_MAIN:
        if (m_position >= 3 && m_position <= 9) {
          bitWrite(m_alarmToCreate.alStruct.dayOfWeek, m_position - 3, !AlarmUtils::isDayWeekEnabled(m_position - 2, m_alarmToCreate));
        } else if (m_position == 10) {
          bitWrite(m_alarmToCreate.alStruct.oddWeeks, 0, !bitRead(m_alarmToCreate.alStruct.oddWeeks, 0));
        } else if (m_position == 11) {
          bitWrite(m_alarmToCreate.alStruct.evenWeeks, 0, !bitRead(m_alarmToCreate.alStruct.evenWeeks, 0));
        } else if (m_position == 12 || m_position == 13) {
          if (m_position == 12) {
            AlarmUtils::addAlarm(m_alarmToCreate);
            CommandProcessor::sendSyncRequest(false);
          }

          AlarmUtils::defaultAlarm(&m_alarmToCreate);
          m_position = 1;

          setMode(MODE_IDLE);
        }

        break;
      default:
        break;
    }

    updateScreen();
  } else if (m_actualMode == MODE_LISTALARM) {
    int alarmCount = AlarmUtils::getAlarmCount();

    switch (action) {
      case ACTION_RIGHT:
        if (m_position < alarmCount)
          m_position++;

        m_secondaryData = 1;
        break;
      case ACTION_LEFT:
        if (m_position > 1)
          m_position--;
        else
          m_actualMode = MODE_IDLE;

        m_secondaryData = 1;
        break;
      case ACTION_UP:
        if (m_secondaryData == 2)
          m_secondaryData = 1;

        break;
      case ACTION_DOWN:
        if (m_secondaryData == 1)
          m_secondaryData = 2;

        break;
      case ACTION_MAIN:
        if (m_secondaryData == 1) {
          AlarmUtils::setAlarmEnabled(m_position, !AlarmUtils::isAlarmEnabled(m_position));
          CommandProcessor::sendSyncRequest(false);
        } else if (m_secondaryData == 2) {
          m_secondaryData = m_position;
          m_position = 1;

          setMode(MODE_DELETEALARM);
        }

        break;
      default:
        break;
    }

    updateScreen();
  } else if (m_actualMode == MODE_NOTIFICATION) {
    switch (action) {
      case ACTION_MAIN:
        setMode(m_modeBeforeNotification);

        break;
    }
  } else if (m_actualMode == MODE_DELETEALARM) {
    switch (action) {
      case ACTION_RIGHT:
        if (m_position == 1)
          m_position = 2;

        break;
      case ACTION_LEFT:
        if (m_position == 2)
          m_position = 1;

        break;
      case ACTION_MAIN:
        if (m_position == 1) {
          AlarmUtils::removeAlarm(m_secondaryData);
          CommandProcessor::sendSyncRequest(false);

          m_position = m_secondaryData;
          m_secondaryData = 1;

          if (AlarmUtils::getAlarmCount() == 0) {
            m_actualMode = MODE_IDLE;
          } else if (m_position > AlarmUtils::getAlarmCount()) {
            m_position = AlarmUtils::getAlarmCount();

            m_actualMode = MODE_LISTALARM;
          } else {
            m_actualMode = MODE_LISTALARM;
          }
        } else if (m_position == 2) {
          m_position = m_secondaryData;
          m_secondaryData = 2;

          m_actualMode = MODE_LISTALARM;
        }

        break;
      default:
        break;
    }

    updateScreen();
  } else if (m_actualMode == MODE_MAINMENU) {
    switch (action) {
      case ACTION_UP:
        if (m_position == 1)
          setMode(MODE_IDLE);
        else
          m_position--;

        break;
      case ACTION_DOWN:
        if (m_position < 3)
          m_position++;

        break;
      case ACTION_MAIN:
        if (m_position == 1)
          setMode(MODE_ALARMCREATE1);
        else if (m_position == 2)
          setMode(MODE_SETCLOCK1);
        else if (m_position == 3)
          setMode(MODE_SETTINGS);
        else
          setMode(MODE_IDLE);

        break;
      default:
        break;
    }

    updateScreen();
  } else if (m_actualMode == MODE_SETTINGS) {
    switch (action) {
      case ACTION_RIGHT:
        if (m_position < 3)
          m_position++;

        break;
      case ACTION_LEFT:
        if (m_position > 1)
          m_position--;

        break;
      case ACTION_MAIN:
        {
          if (m_position == 3) {
            m_position = 1;

            setMode(MODE_IDLE);
          } else {
            Settings::settingBlock1 settingBlock1 = Settings::getSettingBlock1();

            if (m_position == 1)
              settingBlock1.settingBlockStruct.is12HrsFormat = !settingBlock1.settingBlockStruct.is12HrsFormat;
            else if (m_position == 2)
              settingBlock1.settingBlockStruct.isDegreeFahrenheit = !settingBlock1.settingBlockStruct.isDegreeFahrenheit;

            Settings::setSettingBlock1(settingBlock1);
          }

          break;
        }
      default:
        break;
    }

    updateScreen();
  } else if (m_actualMode >= MODE_SETCLOCK1 && m_actualMode <= MODE_SETCLOCK3) {
    switch (action) {
      case ACTION_LEFT:
        if (m_position > 1) {
          m_position--;

          checkPositionAndMode();
        }

        break;
      case ACTION_RIGHT:
        if (m_position < 7) {
          m_position++;

          checkPositionAndMode();
        }

        break;
      case ACTION_UP:
        if (m_position == 1) {
          if (m_clockToSet.hours < 23)
            m_clockToSet.hours++;
          else
            m_clockToSet.hours = 0;
        } else if (m_position == 2) {
          if (m_clockToSet.minutes < 59)
            m_clockToSet.minutes++;
          else
            m_clockToSet.minutes = 0;
        } else if (m_position == 3) {
          if (m_clockToSet.day < Time::getMaxDay(m_clockToSet.month, m_clockToSet.year))
            m_clockToSet.day++;
          else
            m_clockToSet.day = 1;
        } else if (m_position == 4) {
          if (m_clockToSet.month < 12)
            m_clockToSet.month++;
          else
            m_clockToSet.month = 1;

          if (m_clockToSet.day > Time::getMaxDay(m_clockToSet.month, m_clockToSet.year))
            m_clockToSet.day = Time::getMaxDay(m_clockToSet.month, m_clockToSet.year);
        } else if (m_position == 5) {
          if (m_clockToSet.year < 2099)
            m_clockToSet.year++;
          else
            m_clockToSet.year = 2000;

          if (m_clockToSet.day > Time::getMaxDay(m_clockToSet.month, m_clockToSet.year))
            m_clockToSet.day = Time::getMaxDay(m_clockToSet.month, m_clockToSet.year);
        }

        break;
      case ACTION_DOWN:
        if (m_position == 1) {
          if (m_clockToSet.hours > 0)
            m_clockToSet.hours--;
          else
            m_clockToSet.hours = 23;
        } else if (m_position == 2) {
          if (m_clockToSet.minutes > 0)
            m_clockToSet.minutes--;
          else
            m_clockToSet.minutes = 59;
        } else if (m_position == 3) {
          if (m_clockToSet.day > 1)
            m_clockToSet.day--;
          else
            m_clockToSet.day = Time::getMaxDay(m_clockToSet.month, m_clockToSet.year);
        } else if (m_position == 4) {
          if (m_clockToSet.month > 1)
            m_clockToSet.month--;
          else
            m_clockToSet.month = 12;

          if (m_clockToSet.day > Time::getMaxDay(m_clockToSet.month, m_clockToSet.year))
            m_clockToSet.day = Time::getMaxDay(m_clockToSet.month, m_clockToSet.year);
        } else if (m_position == 5) {
          if (m_clockToSet.year > 2000)
            m_clockToSet.year--;
          else
            m_clockToSet.year = 2099;

          if (m_clockToSet.day > Time::getMaxDay(m_clockToSet.month, m_clockToSet.year))
            m_clockToSet.day = Time::getMaxDay(m_clockToSet.month, m_clockToSet.year);
        }

        break;
      case ACTION_MAIN:
        if (m_position == 6 || m_position == 7) {
          if (m_position == 6)
            Time::setRTCTime(m_clockToSet);

          Time::clearTimeStruct(&m_clockToSet);
          m_position = 1;

          setMode(MODE_IDLE);
        }

        break;
      default:
        break;
    }

    updateScreen();
  }
}

void LCDController::updateScreen() {
  m_lcd.clear();

  switch (m_actualMode) {
    case MODE_IDLE:
      {
        updateIdleScreen(m_clock, m_temperature, m_date, true);

        int alarmCount = AlarmUtils::getAlarmCount();

        m_lcd.setCursor(0, 1);
        m_lcd.print(String(alarmCount) + (alarmCount > 1 ? " Alarms" : " Alarm"));

        break;
      }
    case MODE_ALARMCREATE1:
      m_lcd.print("New Alarm");

      m_lcd.setCursor(15, 0);
      m_lcd.write(126);

      m_lcd.setCursor(0, 1);
      m_lcd.print(Time::getFormatted24Hrs(m_alarmToCreate.alStruct.hours, m_alarmToCreate.alStruct.minutes, m_position, Settings::getSettingBlock1().settingBlockStruct.is12HrsFormat));

      break;
    case MODE_ALARMCREATE2:
      m_lcd.print("Repetition");

      m_lcd.setCursor(12, 0);
      m_lcd.write(127);
      m_lcd.write(126);

      m_lcd.setCursor(0, 1);
      m_lcd.print(getStringWithSelector(3, "M"));
      m_lcd.print(getStringWithSelector(4, "T"));
      m_lcd.print(getStringWithSelector(5, "W"));
      m_lcd.print(getStringWithSelector(6, "T"));
      m_lcd.print(getStringWithSelector(7, "F"));
      m_lcd.print(getStringWithSelector(8, "S"));
      m_lcd.print(getStringWithSelector(9, "S"));

      m_lcd.setCursor(15, 0);
      if (AlarmUtils::isDayWeekEnabled(m_position - 2, m_alarmToCreate))
        m_lcd.print("E");
      else
        m_lcd.print("D");

      break;
    case MODE_ALARMCREATE3:
      m_lcd.print("Odd Weeks");

      m_lcd.setCursor(14, 0);
      m_lcd.write(127);
      m_lcd.write(126);

      m_lcd.setCursor(0, 1);
      if (m_alarmToCreate.alStruct.oddWeeks == 1)
        m_lcd.print(getStringWithSelector(10, "Yes"));
      else
        m_lcd.print(getStringWithSelector(10, "No"));

      break;
    case MODE_ALARMCREATE4:
      m_lcd.print("Even Weeks");

      m_lcd.setCursor(14, 0);
      m_lcd.write(127);
      m_lcd.write(126);

      m_lcd.setCursor(0, 1);
      if (m_alarmToCreate.alStruct.evenWeeks == 1)
        m_lcd.print(getStringWithSelector(11, "Yes"));
      else
        m_lcd.print(getStringWithSelector(11, "No"));

      break;
    case MODE_ALARMCREATE5:
      m_lcd.print("Confirmation");

      m_lcd.setCursor(14, 0);
      m_lcd.write(127);

      m_lcd.setCursor(0, 1);
      m_lcd.print(getStringWithSelector(12, "Confirm"));
      m_lcd.print(" ");
      m_lcd.print(getStringWithSelector(13, "Cancel"));

      break;
    case MODE_NOTIFICATION:
      m_lcd.print(m_notification);

      break;
    case MODE_LISTALARM:
      {
        AlarmUtils::alarm readAlarm;
        AlarmUtils::clearAlarm(&readAlarm);
        AlarmUtils::readAlarmFromEEPROM(m_position, &readAlarm);

        if (m_secondaryData == 1) {
          m_lcd.print("Alarm " + String(m_position));

          m_lcd.setCursor(0, 1);
          m_lcd.print(Time::getFormatted24Hrs(readAlarm.alStruct.hours, readAlarm.alStruct.minutes, 0, Settings::getSettingBlock1().settingBlockStruct.is12HrsFormat));

          m_lcd.setCursor(15, 0);
          if (readAlarm.alStruct.isEnabled == 1)
            m_lcd.print("E");
          else
            m_lcd.print("D");
        } else if (m_secondaryData == 2) {
          if (readAlarm.alStruct.evenWeeks == 1 && readAlarm.alStruct.oddWeeks == 1)
            m_lcd.print("Even/Odd Weeks");
          else if (readAlarm.alStruct.evenWeeks == 1 && readAlarm.alStruct.oddWeeks == 0)
            m_lcd.print("Even Weeks only");
          else if (readAlarm.alStruct.evenWeeks == 0 && readAlarm.alStruct.oddWeeks == 1)
            m_lcd.print("Odd Weeks only");
          else
            m_lcd.print("Never");

          m_lcd.setCursor(0, 1);
          char *weekDays[7] = {"M", "T", "W", "T", "F", "S", "S"};
          for (int i = 1; i <= 7; i++) {
            if (AlarmUtils::isDayWeekEnabled(i, readAlarm)) {
              m_lcd.print(weekDays[i - 1]);
            } else {
              m_lcd.print("-");
            }
          }
        }
      }

      break;
    case MODE_DELETEALARM:
      m_lcd.print("Delete Alarm " + String(m_secondaryData));

      m_lcd.setCursor(0, 1);
      m_lcd.print(getStringWithSelector(1, "Confirm"));
      m_lcd.print(" ");
      m_lcd.print(getStringWithSelector(2, "Cancel"));

      break;
    case MODE_MAINMENU:
      m_lcd.print("Menu");

      m_lcd.setCursor(13, 0);
      m_lcd.print(String(m_position) + "/3");

      m_lcd.setCursor(0, 1);
      if (m_position == 1)
        m_lcd.print("> Add an Alarm");
      else if (m_position == 2)
        m_lcd.print("> Set Clock");
      else if (m_position == 3)
        m_lcd.print("> Settings");

      break;
    case MODE_SETTINGS:
      {
        Settings::settingBlock1 settingBlock1 = Settings::getSettingBlock1();
        m_lcd.print("Settings - ");

        if (m_position == 1) {
          m_lcd.print("24Hrs");
        } else if (m_position == 2) {
          m_lcd.write(223);
          m_lcd.print("C");
        } else if (m_position == 3) {
          m_lcd.print("Exit");
        }

        m_lcd.setCursor(0, 1);
        if (m_position == 1) {
          if (settingBlock1.settingBlockStruct.is12HrsFormat)
            m_lcd.print(getStringWithSelector(1, "No"));
          else
            m_lcd.print(getStringWithSelector(1, "Yes"));
        } else if (m_position == 2) {
          if (settingBlock1.settingBlockStruct.isDegreeFahrenheit)
            m_lcd.print(getStringWithSelector(2, "No"));
          else
            m_lcd.print(getStringWithSelector(2, "Yes"));
        }

        break;
      }
    case MODE_SETCLOCK1:
      m_lcd.print("Time");

      m_lcd.setCursor(15, 0);
      m_lcd.write(126);

      m_lcd.setCursor(0, 1);
      m_lcd.print(Time::getFormatted24Hrs(m_clockToSet.hours, m_clockToSet.minutes, m_position, Settings::getSettingBlock1().settingBlockStruct.is12HrsFormat));

      break;
    case MODE_SETCLOCK2:
      m_lcd.print("Date");

      m_lcd.setCursor(14, 0);
      m_lcd.write(127);
      m_lcd.write(126);

      m_lcd.setCursor(0, 1);
      m_lcd.print(getStringWithSelector(3, Time::formatNumber(m_clockToSet.day)));
      m_lcd.print("/");
      m_lcd.print(getStringWithSelector(4, Time::formatNumber(m_clockToSet.month)));
      m_lcd.print("/");
      m_lcd.print(getStringWithSelector(5, Time::formatNumber(m_clockToSet.year)));

      break;
    case MODE_SETCLOCK3:
      m_lcd.print("Confirmation");

      m_lcd.setCursor(14, 0);
      m_lcd.write(127);

      m_lcd.setCursor(0, 1);
      m_lcd.print(getStringWithSelector(6, "Confirm"));
      m_lcd.print(" ");
      m_lcd.print(getStringWithSelector(7, "Cancel"));

      break;
    default:
      break;
  }
}

void LCDController::update() {
  if (m_actualMode == MODE_NOTIFICATION) {
    if (millis() >= m_endTimeNotification) {
      setMode(m_modeBeforeNotification);
    } else {
      m_lcd.setCursor(0, 1);
      m_lcd.print("[");
      m_lcd.setCursor(15, 1);
      m_lcd.print("]");

      unsigned int timePassed = millis() - m_startTimeNotification;
      unsigned int notificationDuration = m_endTimeNotification - m_startTimeNotification;
      float timePassedChar = (float)(timePassed * 14.0) / (float)notificationDuration;

      if (timePassedChar > 14)
        timePassedChar = 14;

      String charToPrint = "";
      for (int i = 0; i < timePassedChar; i++) {
        charToPrint += "=";
      }

      m_lcd.setCursor(1, 1);
      m_lcd.print(charToPrint);

      m_oldTimePassedChar = timePassedChar;
    }
  }

  if ((m_actualMode == MODE_MAINMENU || m_actualMode == MODE_LISTALARM) && millis() >= m_lastEvent + 60000)
    setMode(MODE_IDLE);
}

void LCDController::sendNotification(String message, unsigned long notificationDuration) {
  m_notification = message;
  m_startTimeNotification = millis();
  m_endTimeNotification = m_startTimeNotification + notificationDuration;
  m_oldTimePassedChar = -1;
  m_modeBeforeNotification = m_actualMode;

  setMode(MODE_NOTIFICATION);
}

void LCDController::setIdleMessage(String clock, String temperature, String date) {
  if (m_actualMode == MODE_IDLE)
    updateIdleScreen(clock, temperature, date);
    
  m_clock = clock;
  m_temperature = temperature;
  m_date = date;
}

void LCDController::setMode(DisplayMode newMode) {
  m_actualMode = newMode;

  if (newMode == MODE_ALARMCREATE1)
    AlarmUtils::defaultAlarm(&m_alarmToCreate);
  else if (newMode == MODE_SETCLOCK1)
    m_clockToSet = Time::getRTCTime();

  if (newMode == MODE_ALARMCREATE1 || newMode == MODE_LISTALARM || newMode == MODE_MAINMENU || newMode == MODE_SETTINGS || newMode == MODE_SETCLOCK1)
    m_position = 1;

  updateScreen();
}

LCDController::DisplayMode LCDController::getMode() {
  return m_actualMode;
}

int LCDController::getPosition() {
  return m_position;
}

void LCDController::setPosition(int position) {
  m_position = position;
}

int LCDController::getSecondaryData() {
  return m_secondaryData;
}

void LCDController::setSecondaryData(int secondaryData) {
  m_secondaryData = secondaryData;
}

String LCDController::getStringWithSelector(int pos, String text) {
  if (pos == m_position)
    return "[" + text + "]";
  else
    return text;
}

void LCDController::checkPositionAndMode() {
  if (m_actualMode >= MODE_ALARMCREATE1 && m_actualMode <= MODE_ALARMCREATE5) {
    if ((m_position == 1 || m_position == 2) && m_actualMode != MODE_ALARMCREATE1)
      m_actualMode = MODE_ALARMCREATE1;
    else if ((m_position >= 3 && m_position <= 9) && m_actualMode != MODE_ALARMCREATE2)
      m_actualMode = MODE_ALARMCREATE2;
    else if (m_position == 10 && m_actualMode != MODE_ALARMCREATE3)
      m_actualMode = MODE_ALARMCREATE3;
    else if (m_position == 11 && m_actualMode != MODE_ALARMCREATE4)
      m_actualMode = MODE_ALARMCREATE4;
    else if ((m_position == 12 || m_position == 13) && m_actualMode != MODE_ALARMCREATE5)
      m_actualMode = MODE_ALARMCREATE5;
  } else if (m_actualMode >= MODE_SETCLOCK1 && m_actualMode <= MODE_SETCLOCK3) {
    if ((m_position == 1 || m_position == 2) && m_actualMode != MODE_SETCLOCK1)
      m_actualMode = MODE_SETCLOCK1;
    else if ((m_position >= 3 && m_position <= 5) && m_actualMode != MODE_SETCLOCK2)
      m_actualMode = MODE_SETCLOCK2;
    else if ((m_position == 6 || m_position == 7) && m_actualMode != MODE_SETCLOCK3)
      m_actualMode = MODE_SETCLOCK3;
  }
}

void LCDController::updateIdleScreen(String clock, String temperature, String date, bool forceUpdate) {
  m_lcd.setCursor(0, 0);
  
  if (m_position == 1 && (clock != m_clock || forceUpdate))
    m_lcd.print(clock);
  else if (m_position == 2 && (date != m_date || forceUpdate))
    m_lcd.print(date);

  if (temperature.length() && (temperature != m_temperature || forceUpdate)) {
    int clkLength = m_position == 1 ? clock.length() : date.length();
    
    m_lcd.setCursor(clkLength, 0);
    for (int i = clkLength; i <= 15; i++)
      m_lcd.print(' ');
    
    m_lcd.setCursor(15 - temperature.length(), 0);
    m_lcd.print(temperature);

    m_lcd.setCursor(15, 0);
    if (Settings::getSettingBlock1().settingBlockStruct.isDegreeFahrenheit)
      m_lcd.write(byte(CHAR_DEGREEFAHRENHEIT));
    else
      m_lcd.write(byte(CHAR_DEGREECELSIUS));
  }
}

