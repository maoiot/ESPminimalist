#include <WiFiClientSecure.h>
#include "ESPConfig.h"

#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H
class TelegramBot {
  private:
    WiFiClientSecure _clnt;
    String _botid;
    bool   _sendCommand(const char *cmd);
    String _result;
    bool _keepALive;
  public:
    TelegramBot(const char * botid = "", bool keepALive = true);
    bool   begin(ESPConfig *cfg);
    bool   sendMsg(const char *chat_id, const char *text);
    String getMe();
    String getResult() { return _result; }
};
#endif
