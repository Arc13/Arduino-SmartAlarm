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

      REQUEST_SETSETTING,

      REQUEST_GETALARMCOUNT,
      REQUEST_GETALARM,

      REQUEST_GETTIME,
      REQUEST_GETTEMP,
      REQUEST_SETTIME,
    };

    static void processCommand(String command, bool *updateScreen, RequestType *requestType);

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

    static bool verifyArgCount(size_t size, size_t expectedSize);

    static void sendResponse(ResponseType responseType);
    static void sendResponse(ResponseType responseType, int dataLength, char *data[]);
    static String adaptDataArg(char *dataArg);
};

#endif
