#ifndef CMDPROCESSOR_H
#define CMDPROCESSOR_H

#include "Arduino.h"

class CommandProcessor {
  public:
    enum RequestType {
      REQUEST_ADDALARM = 1,
      REQUEST_REMALARM,
      REQUEST_MODALARM,

      REQUEST_ENALARM,
      REQUEST_DISALARM,

      REQUEST_GETSETTINGS,
      REQUEST_SETSETTING,

      REQUEST_GETALARMCOUNT,
      REQUEST_GETALARM,

      REQUEST_GETTIME,
      REQUEST_GETTEMP,
      REQUEST_SETTIME,

      REQUEST_SYNC,
      REQUEST_SYNC_LOW,

      REQUEST_SYNC_ALARM_REMOVE,
      REQUEST_SYNC_ALARM_STATE,
    };

    static void processCommand(String command, bool *updateScreen, RequestType *requestType);

    static void sendSyncRequest(boolean low);
    static void syncAlarmRemove(int position);
    static void syncAlarmState(int position, boolean state);

  private:
    enum ResponseType {
      RESPONSE_ERROR = 0,
      RESPONSE_OK = 1,

      RESPONSE_WRONGARG_TYPE,
      RESPONSE_WRONGARG_COUNT,

      RESPONSE_ALARMNOTFOUND,

      RESPONSE_SETTINGNOTFOUND,

      RESPONSE_RTCNOTAVAILABLE,

      RESPONSE_MISSINGVALUES,
      RESPONSE_INVALIDJSON,
    };

    static bool verifyArgCount(size_t size, size_t expectedSize, RequestType requestType);

    static void sendResponse(ResponseType responseType, RequestType requestType);
    static void sendResponse(ResponseType responseType, RequestType requestType, int dataLength, char *data[]);
    static String adaptDataArg(char *dataArg);
};

#endif
