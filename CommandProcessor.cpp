#include <ArduinoJson.h>
#include "AlarmUtilities.h"
#include "CommandProcessor.h"
#include "Settings.h"
#include "Time.h"

void CommandProcessor::processCommand(String command, bool *updateScreen, RequestType *requestType) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(command);

  if (root.success()) {
    if (root["rqt"].is<int>() && root["data"].is<JsonArray>()) {
      RequestType rqt = root["rqt"].as<int>();
      *requestType = rqt;

      switch (rqt) {
        case REQUEST_ADDALARM:
          {
            if (!verifyArgCount(root["data"].size(), 11, rqt))
              return;

            for (int i = 0; i < 11; i++) {
              if (!root["data"][i].is<int>()) {
                sendResponse(RESPONSE_WRONGARG_TYPE, rqt);
                return;
              }
            }

            AlarmUtils::alarm alarmToAdd;
            AlarmUtils::defaultAlarm(&alarmToAdd);

            alarmToAdd.alStruct.hours = root["data"][0];
            alarmToAdd.alStruct.minutes = root["data"][1];

            for (int i = 0; i < 7; i++)
              bitWrite(alarmToAdd.alStruct.dayOfWeek, i, root["data"][i + 2]);

            alarmToAdd.alStruct.evenWeeks = root["data"][9];
            alarmToAdd.alStruct.oddWeeks = root["data"][10];

            AlarmUtils::addAlarm(alarmToAdd);

            break;
          }
        case REQUEST_REMALARM:
          {
            if (!verifyArgCount(root["data"].size(), 1, rqt))
              return;

            if (!root["data"][0].is<int>()) {
              sendResponse(RESPONSE_WRONGARG_TYPE, rqt);
              return;
            }

            if (root["data"][0] > AlarmUtils::getAlarmCount()) {
              sendResponse(RESPONSE_ALARMNOTFOUND, rqt);
              return;
            }

            AlarmUtils::removeAlarm(root["data"][0]);

            break;
          }
        case REQUEST_DISALARM:
        // Fall into REQUEST_ENALARM
        case REQUEST_ENALARM:
          {
            if (!verifyArgCount(root["data"].size(), 1, rqt))
              return;

            if (!root["data"][0].is<int>()) {
              sendResponse(RESPONSE_WRONGARG_TYPE, rqt);
              return;
            }

            if (root["data"][0] > AlarmUtils::getAlarmCount()) {
              sendResponse(RESPONSE_ALARMNOTFOUND, rqt);
              return;
            }

            AlarmUtils::setAlarmEnabled(root["data"][0], rqt == REQUEST_ENALARM);

            break;
          }
        case REQUEST_MODALARM:
          {
            if (!verifyArgCount(root["data"].size(), 12, rqt))
              return;

            for (int i = 0; i < 12; i++) {
              if (!root["data"][i].is<int>()) {
                sendResponse(RESPONSE_WRONGARG_TYPE, rqt);
                return;
              }
            }

            if (root["data"][0] > AlarmUtils::getAlarmCount()) {
              sendResponse(RESPONSE_ALARMNOTFOUND, rqt);
              return;
            }

            AlarmUtils::alarm alarmToModify;
            AlarmUtils::readAlarmFromEEPROM(root["data"][0], &alarmToModify);

            alarmToModify.alStruct.hours = root["data"][1];
            alarmToModify.alStruct.minutes = root["data"][2];

            for (int i = 0; i < 7; i++)
              bitWrite(alarmToModify.alStruct.dayOfWeek, i, root["data"][i + 3]);

            alarmToModify.alStruct.evenWeeks = root["data"][10];
            alarmToModify.alStruct.oddWeeks = root["data"][11];

            AlarmUtils::writeAlarmToEEPROM(root["data"][0], alarmToModify);

            break;
          }
        case REQUEST_GETSETTINGS:
          {
            Settings::settingBlock1 settingBlock = Settings::getSettingBlock1();
            
            char hourFormatBuffer[2];
            char temperatureFormatBuffer[2];
            itoa(bitRead(settingBlock.settingBlockByte[0], 0), hourFormatBuffer, 10);
            itoa(bitRead(settingBlock.settingBlockByte[0], 1), temperatureFormatBuffer, 10);
            char *data[] = {hourFormatBuffer, temperatureFormatBuffer};

            sendResponse(RESPONSE_OK, rqt, 2, data);
            return;
             
            break;
          }
        case REQUEST_SETSETTING:
          {
            if (!verifyArgCount(root["data"].size(), 2, rqt))
              return;

            for (int i = 0; i < 2; i++) {
              if (!root["data"][i].is<int>()) {
                sendResponse(RESPONSE_WRONGARG_TYPE, rqt);
                return;
              }
            }

            if (root["data"][0] > 2) {
              sendResponse(RESPONSE_SETTINGNOTFOUND, rqt);
              return;
            }

            Settings::settingBlock1 settingBlock = Settings::getSettingBlock1();
            bitWrite(settingBlock.settingBlockByte[0], root["data"][0].as<byte>() - 1, root["data"][1].as<byte>());
            Settings::setSettingBlock1(settingBlock);
            settingBlock = Settings::getSettingBlock1();

            break;
          }
        case REQUEST_GETALARMCOUNT:
          {
            char alarmCountArray[4];
            itoa(AlarmUtils::getAlarmCount(), alarmCountArray, 10);

            char *data[] = {alarmCountArray};
            sendResponse(RESPONSE_OK, rqt, 1, data);
            return;

            break;
          }
        case REQUEST_GETALARM:
          {
            if (!verifyArgCount(root["data"].size(), 1, rqt))
              return;

            if (!root["data"][0].is<int>()) {
              sendResponse(RESPONSE_WRONGARG_TYPE, rqt);
              return;
            }

            if (root["data"][0] > AlarmUtils::getAlarmCount() || root["data"][0] < 1) {
              sendResponse(RESPONSE_ALARMNOTFOUND, rqt);
              return;
            }

            AlarmUtils::alarm alarmToRead;
            AlarmUtils::readAlarmFromEEPROM(root["data"][0], &alarmToRead);

            char hoursBuffer[3];
            char minutesBuffer[3];
            char dOWBuffer[4];
            char evenWeeksBuffer[2];
            char oddWeeksBuffer[2];
            char isEnabledBuffer[2];
            itoa(alarmToRead.alStruct.hours, hoursBuffer, 10);
            itoa(alarmToRead.alStruct.minutes, minutesBuffer, 10);
            itoa(alarmToRead.alStruct.dayOfWeek, dOWBuffer, 10);
            itoa(alarmToRead.alStruct.evenWeeks, evenWeeksBuffer, 10);
            itoa(alarmToRead.alStruct.oddWeeks, oddWeeksBuffer, 10);
            itoa(alarmToRead.alStruct.isEnabled, isEnabledBuffer, 10);
            char *data[] = {root["data"][0], hoursBuffer, minutesBuffer, dOWBuffer, evenWeeksBuffer, oddWeeksBuffer, isEnabledBuffer};

            sendResponse(RESPONSE_OK, rqt, 7, data);
            return;

            break;
          }
        case REQUEST_GETTIME:
          {
            Time::timeStruct actualTime = Time::getRTCTime();

            char hoursBuffer[3];
            char minutesBuffer[3];
            char secondsBuffer[3];
            char dateBuffer[3];
            char monthBuffer[3];
            char yearBuffer[5];
            itoa(actualTime.hours, hoursBuffer, 10);
            itoa(actualTime.minutes, minutesBuffer, 10);
            itoa(actualTime.seconds, secondsBuffer, 10);
            itoa(actualTime.day, dateBuffer, 10);
            itoa(actualTime.month, monthBuffer, 10);
            itoa(actualTime.year, yearBuffer, 10);
            char *data[] = {hoursBuffer, minutesBuffer, secondsBuffer, dateBuffer, monthBuffer, yearBuffer};

            sendResponse(RESPONSE_OK, rqt, 6, data);
            return;

            break;
          }
        case REQUEST_GETTEMP:
          {
            RtcDS3231<TwoWire> rtc(Wire);
            rtc.Begin();

            if (rtc.GetIsRunning()) {
              RtcTemperature actualTemperature = rtc.GetTemperature();

              char tempWholeBuffer[4];
              char tempFractBuffer[4];
              itoa(actualTemperature.AsWholeDegrees(), tempWholeBuffer, 10);
              itoa(actualTemperature.GetFractional(), tempFractBuffer, 10);
              char *data[] = {tempWholeBuffer, tempFractBuffer};

              sendResponse(RESPONSE_OK, rqt, 2, data);
              return;
            } else {
              sendResponse(RESPONSE_RTCNOTAVAILABLE, rqt);
              return;
            }

            break;
          }
        case REQUEST_SETTIME:
          {
            if (!verifyArgCount(root["data"].size(), 6, rqt))
              return;

            for (int i = 0; i < 6; i++) {
              if (!root["data"][i].is<int>()) {
                sendResponse(RESPONSE_WRONGARG_TYPE, rqt);
                return;
              }
            }

            Time::timeStruct timeToSet;
            Time::clearTimeStruct(&timeToSet);

            timeToSet.hours = root["data"][0].as<int>();
            timeToSet.minutes = root["data"][1].as<int>();
            timeToSet.seconds = root["data"][2].as<int>();
            timeToSet.day = root["data"][3].as<int>();
            timeToSet.month = root["data"][4].as<int>();
            timeToSet.year = root["data"][5].as<int>();

            if (!Time::setRTCTime(timeToSet)) {
              sendResponse(RESPONSE_RTCNOTAVAILABLE, rqt);
              return;
            }

            break;
          }
        default:
          break;
      }


      sendResponse(RESPONSE_OK, rqt);
    } else {
      sendResponse(RESPONSE_MISSINGVALUES, -1);
      return;
    }
  } else {
    sendResponse(RESPONSE_INVALIDJSON, -1);
    return;
  }

  *updateScreen = true;
}

bool CommandProcessor::verifyArgCount(size_t size, size_t expectedSize, RequestType requestType) {
  if (size != expectedSize) {
    sendResponse(RESPONSE_WRONGARG_COUNT, requestType);
    return false;
  } else {
    return true;
  }
}

void CommandProcessor::sendResponse(ResponseType responseType, RequestType requestType) {
  Serial.println("{\"rpt\":" + String(responseType) + ", \"rqt\":" + String(requestType) + "}");
}

void CommandProcessor::sendResponse(ResponseType responseType, RequestType requestType, int dataLength, char *data[]) {
  String responseToSend = "{\"rpt\":" + String(responseType) + ", \"rqt\":" + String(requestType) + ", \"data\":[";

  for (int i = 0; i < dataLength - 1; i++) {
    responseToSend += adaptDataArg(data[i]);
    responseToSend += ", ";
  }
  responseToSend += adaptDataArg(data[dataLength - 1]);

  responseToSend += "]}";

  Serial.println(responseToSend);
}

String CommandProcessor::adaptDataArg(char *dataArg) {
  if (atoi(dataArg) || strcmp(dataArg, "0") == 0)
    return String(dataArg);
  else
    return String("\"" + String(dataArg) + "\"");
}

void CommandProcessor::sendSyncRequest(boolean low) {
  if (low) {
    sendResponse(RESPONSE_OK, REQUEST_SYNC_LOW);
  } else {
    sendResponse(RESPONSE_OK, REQUEST_SYNC);
  }
}

