#include "TelegramBot.h"

#define SSL_PORT 443

//#define LOG

#ifdef LOG
const char* c_log_format PROGMEM = "%s(%d) free heap %d\r\n";
#define LOGW(str) Serial.printf_P(c_log_format, str, __LINE__, ESP.getFreeHeap())
#else
#define LOGW(str)
#endif

const char* c_tgmhost PROGMEM = "api.telegram.org";
const char* c_sendmsg_header PROGMEM = "POST /bot%s/sendMessage HTTP/1.1\r\nHost: %s\r\nUser-Agent: BOT\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n{\"chat_id\":\"%s\",\"text\":\"%s\"}\r\n";

TelegramBot::TelegramBot(const char * botid, bool keepALive) {
  _botid = botid;
  _keepALive = keepALive;
}

bool TelegramBot::begin(ESPConfig *cfg) {
  if ( cfg ) {
    if ( cfg->isServiceMode() ) {
      _botid = "";
      _keepALive = false;
    } else {
      _botid = cfg->get("bot_id");
      _keepALive = ( cfg->get("bot_keepalive") == "X" );
    }
    return true;
  } return false;
}

bool TelegramBot::sendMsg(const char *chat_id, const char *text)  {

  size_t buf_size = strlen(c_sendmsg_header) + strlen(_botid.c_str()) + strlen(c_tgmhost) + strlen(chat_id) + strlen(text) + 50;
  char * buf = (char *)malloc(buf_size);

  if ( buf != NULL && _botid != "" ) {
    if (_clnt.connect(c_tgmhost, SSL_PORT)) {
      int i = snprintf(buf, buf_size, c_sendmsg_header, _botid.c_str(), c_tgmhost, strlen(chat_id) + strlen(text) + 24, chat_id, text); buf[i] = '\0';
      _clnt.write((const uint8_t*)&buf[0], strlen(buf));
      free(buf);
      buf = NULL;
      String answer;
      while (_clnt.connected()) {
        answer = _clnt.readStringUntil('\n');
        if ( answer == "\r" ) {
          delay(0);
          break;
        }
      }
      while (_clnt.connected()) {
        answer = _clnt.readStringUntil('\n');
        delay(0);
        break;
      }
      _result = answer;
      if ( !_keepALive ) {
        _clnt.stop();
        delay(50);
      }
      return true;
    }
  }
  return false;
}

const char* c_http_tgmheader PROGMEM = "GET /bot%s/%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: BOT\r\nConnection: keep-alive\r\n\r\n";
bool TelegramBot::_sendCommand(const char *cmd) {

  LOGW("_sendCommand");
  size_t buf_size = strlen(c_http_tgmheader) + strlen(_botid.c_str()) + strlen(cmd) + strlen(c_tgmhost) + 1;
  char * buf = (char *)malloc(buf_size);
  LOGW("_sendCommand");
  if ( buf != NULL ) {
    LOGW("_sendCommand");
    if (_clnt.connect(c_tgmhost, SSL_PORT)) {
      LOGW("_sendCommand");
      int i = snprintf(buf, buf_size, c_http_tgmheader, _botid.c_str(), cmd, c_tgmhost); buf[i] = '\0';
      _clnt.write((const uint8_t*)&buf[0], strlen(buf));
      LOGW("_sendCommand");
      free(buf);
      buf = NULL;
      String answer;
      while (_clnt.connected()) {
        answer = _clnt.readStringUntil('\n');
        if ( answer == "\r" ) {
          delay(0);
          break;
        }
      }
      while (_clnt.connected()) {
        answer = _clnt.readStringUntil('\n');
        delay(0);
        break;
      }
      if ( !_keepALive ) {
        _clnt.stop();
        delay(50);
      }
      _result = answer;
      LOGW("_sendCommand");
      return true;
    }
  }
  return false;
}

const char* c_cmd_getme PROGMEM = "getMe";
String TelegramBot::getMe() {
  if ( _sendCommand(c_cmd_getme) ) return _result;
  else return "";
}

