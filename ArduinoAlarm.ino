#define BUZZERPIN 3

#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include "AlarmUtilities.h"
#include "ButtonController.h"
#include "CommandProcessor.h"
#include "LCDController.h"
#include "SerialUtilities.h"
#include "Settings.h"
#include "Time.h"

unsigned long lastHeldTime = 0, alarmTime = -1;
bool isRTCAvailable = false;

LCDController lcd(8, 9, 4, 5, 6, 7, 16, 2);
ButtonController btnLCD(analogRead(A0));
SerialUtils serial;
Time timeManager;
RtcDS3231<TwoWire> rtc(Wire);

void updateIdleMessage() {
  Time::timeStruct actualTime = Time::rtcDateTimeToTimeStruct(rtc.GetDateTime());

  String actualTimeStr = Time::getFormatted24Hrs(actualTime.hours, actualTime.minutes, 0, Settings::getSettingBlock1().settingBlockStruct.is12HrsFormat, actualTime.seconds % 2);

  RtcTemperature actualTemperature = rtc.GetTemperature();
  float temperature = actualTemperature.AsFloat();
  if (Settings::getSettingBlock1().settingBlockStruct.isDegreeFahrenheit) {
    temperature *= 1.8;
    temperature += 32;
  }
  
  String actualTemperatureStr = String((int)temperature);
  if (temperature - (int)temperature >= 0.5)
    actualTemperatureStr += ".5";

  String actualDateStr = Time::formatNumber(actualTime.day) + "/" + Time::formatNumber(actualTime.month) + "/" + Time::formatNumber(actualTime.year);

  lcd.setIdleMessage(actualTimeStr, actualTemperatureStr, actualDateStr);
}

void onSecondChanged(Time::timeStruct actualTime) {
  updateIdleMessage();
}

void onMinuteChanged(Time::timeStruct actualTime) {
  onSecondChanged(actualTime);

  if (AlarmUtils::isAlarmRunning(actualTime)) {
    lcd.sendNotification("Alarm!", 60000);

    tone(BUZZERPIN, 12000);
    alarmTime = millis();
  }
}

void setup() {
  serial.begin(115200);

  timeManager.setOnSecondChangedListener(&onSecondChanged);
  timeManager.setOnMinuteChangedListener(&onMinuteChanged);

  rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!rtc.IsDateTimeValid())
    rtc.SetDateTime(compiled);

  if (!rtc.GetIsRunning())
    rtc.SetIsRunning(true);

  RtcDateTime now = rtc.GetDateTime();
  if (now < compiled)
    rtc.SetDateTime(compiled);

  rtc.Enable32kHzPin(false);
  rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  if (!rtc.GetIsRunning())
    lcd.setIdleMessage("--:--", "--", "--/--/----");
}

void loop() {
  btnLCD.update(analogRead(A0));
  lcd.update();
  serial.update();

  if (rtc.GetIsRunning())
    timeManager.update(Time::rtcDateTimeToTimeStruct(rtc.GetDateTime()));

  ButtonController::Buttons lastPressedButton = btnLCD.getLastPressedButton();
  ButtonController::Buttons heldButton = btnLCD.getHeldButton();

  if (serial.available()) {
    bool updateScreen = false;
    CommandProcessor::RequestType requestType;
    CommandProcessor::processCommand(serial.receive(), &updateScreen, &requestType);

    if (updateScreen) {
      if (requestType == CommandProcessor::REQUEST_REMALARM) {
        int alarmCount = AlarmUtils::getAlarmCount();

        if (lcd.getMode() == lcd.MODE_LISTALARM) {
          if (alarmCount == 0) {
            lcd.setMode(lcd.MODE_IDLE);
          } else if (lcd.getPosition() > alarmCount) {
            lcd.setPosition(alarmCount);
          }
        } else if (lcd.getMode() == lcd.MODE_DELETEALARM) {
          if (alarmCount == 0) {
            lcd.setMode(lcd.MODE_IDLE);
          } else if (lcd.getSecondaryData() > alarmCount) {
            lcd.setMode(lcd.MODE_LISTALARM);

            lcd.setPosition(alarmCount);
            lcd.setSecondaryData(1);
          }
        }
      }

      lcd.updateScreen();
    }
  }

  lcd.actionHandler((LCDController::Actions)lastPressedButton);

  if (millis() >= alarmTime + 60000 || (lastPressedButton == btnLCD.BUTTON_MAIN && millis() >= alarmTime && millis() <= alarmTime + 60000)) {
    noTone(BUZZERPIN);
    alarmTime = -1;
  }

  if (millis() >= lastHeldTime + 200 && heldButton != btnLCD.BUTTON_MAIN && heldButton != btnLCD.BUTTON_NONE) {
    lastHeldTime = millis();
    lcd.actionHandler((LCDController::Actions)heldButton);
  }
}
