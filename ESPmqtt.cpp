#include "ESPmqtt.h"

#ifdef ESP8266
#include <ESP8266httpUpdate.h>
#else
#include <WiFi.h>
#endif

ESPmqtt::ESPmqtt(const char *prfx) {
  _clnt = new WiFiClient();
  _mqtt = new PubSubClient(*_clnt);

  if ( prfx == NULL ) {
    _mqtt_prfx = PSTR("/ESP") + String(ESP.getChipId());
  }
  else {
    _mqtt_prfx = prfx;
  }
}
ESPmqtt::~ESPmqtt() {
  delete _mqtt;
  delete _clnt;
}

const char* c_sts_online = "online";

void ESPmqtt::begin(ESPConfig *cfg) {
  if ( cfg ) {
    if (cfg->isServiceMode()) {
      _mqtt->setServer(IPAddress(192, 168, 4, 3), 1883); // Fixed IP & port in ServiceMode
      _user = "";
      _upwd = "";
    } else {
      uint16_t port = strtol(cfg->get("mqtt_port").c_str(), NULL, 10);
      IPAddress mqttIP;
      if ( !mqttIP.fromString(cfg->get("mqtt_server")) )
        _mqtt->setServer(cfg->get("mqtt_server").c_str(), port);
      else
        _mqtt->setServer(mqttIP, port);
      _user = cfg->get("mqtt_user");
      _upwd = cfg->get("mqtt_pass");
      if ( cfg->get("mqtt_prefix") != "" )
        _mqtt_prfx = cfg->get("mqtt_prefix");
    }
    //Serial.printf("MQTT:\r\nServer %s, port %d\r\nUser %s\r\nPass %s\r\nPrefix %s\r\n", cfg->get("mqtt_server").c_str(), strtol(cfg->get("mqtt_port").c_str(), NULL, 10), _user.c_str(), _upwd.c_str(), _mqtt_prfx.c_str());
  }
  this->connect();
  _subscribe();
  _mqtt->publish(_mqtt_prfx.c_str(), c_sts_online);
}

void ESPmqtt::begin(const char *server, const int port) {
  _mqtt->setServer(server, port);
}

bool ESPmqtt::connect() {
  if ( !_user.length() )
    return _mqtt->connect(String(ESP.getChipId()).c_str());
  else
    return _mqtt->connect(String(ESP.getChipId()).c_str(), _user.c_str(), _upwd.c_str());
}

bool ESPmqtt::connect(const char *user, const char *upwd) {
  _user = user;
  _upwd = upwd;
  return this->connect();
}

const char *c_config     = "/config";
const char *c_config_all = "/config/#";
const char *c_config_get = "/config/get";
const char *c_config_set = "/config/set";
const char *c_config_sts = "/config/status";

const char *c_flash      = "/flash";
const char *c_flash_sts  = "/flash/status";

const char *c_upd_err    = "invalid (host,port,uri)";
const char *c_upd_failed = "HTTP_UPDATE_FAILD Error %d";
const char *c_upd_noupd  = "HTTP_UPDATE_NO_UPDATES";
const char *c_upd_ok     = "HTTP_UPDATE_OK";

const char *c_reset      = "/reset";
const char *c_reset_sts  = "/reset/status";

const char *c_sts_ok     = "ok";
const char *c_sts_err    = "err";

void ESPmqtt::_callback(char* ptopic, byte* ppayload, unsigned int plength)
{
  ESPConfig cfg;
  String str;
  t_httpUpdate_return ret;

  /*Serial.print(ptopic); // выводим в сериал порт название топика
  Serial.println(" => ");
  Serial.write(ppayload, plength); // выводим в сериал порт значение полученных данных
  Serial.println();*/

  if ( String(ptopic) == _mqtt_prfx + c_config_get )
    if ( cfg.loadCfg() ) {
      str = _mqtt_prfx + c_config;
      _mqtt->publish(str.c_str(), cfg.getCfg().c_str());
      return;
    }
  if ( String(ptopic) == _mqtt_prfx + c_config_set ) {
    cfg.setCfg((char *)ppayload, (size_t)plength);
    str = _mqtt_prfx + c_config_sts;
    if ( cfg.saveCfg() )
      _mqtt->publish(str.c_str(), c_sts_ok);
    else
      _mqtt->publish(str.c_str(), c_sts_err);
    return;
  }
  if ( String(ptopic) == _mqtt_prfx + c_flash ) {
    str = _mqtt_prfx + c_flash_sts;
    char buf[plength + 1];
    memcpy(buf, ppayload, plength);
    buf[plength] = '\0';
    String host = strtok(buf, ",");
    uint16_t port = strtol(strtok(NULL, ","), NULL, 10);
    String uuri = strtok(NULL, ",");

    //Serial.printf("\r\nFlash %s, %d, %s\r\n", host.c_str(), port, uuri.c_str());

    if ( host != "" && port != 0 && uuri != "" )
      ret = ESPhttpUpdate.update(host, port, uuri);
    else {
      _mqtt->publish(str.c_str(), c_upd_err);
      return;
    }
    char sts[30];
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        snprintf(sts, 30, c_upd_failed, ESPhttpUpdate.getLastError());
        break;
      case HTTP_UPDATE_NO_UPDATES:
        snprintf(sts, 30, c_upd_noupd);
        break;

      case HTTP_UPDATE_OK:
        snprintf(sts, 30, c_upd_ok);
        break;
    }

    _mqtt->publish(str.c_str(), sts);
    yield();
    return;
  }
  if ( String(ptopic) == _mqtt_prfx + c_reset ) {
    str = _mqtt_prfx + c_reset_sts;
    _mqtt->publish(str.c_str(), c_sts_ok);
    yield();
    ESP.reset();
    return;
  }
}
void ESPmqtt::_subscribe() {
  String str = _mqtt_prfx + c_config_all;
  _mqtt->subscribe(str.c_str());
  str = _mqtt_prfx + c_flash;
  _mqtt->subscribe(str.c_str());
  str = _mqtt_prfx + c_reset;
  _mqtt->subscribe(str.c_str());
  std::function<void(char*, uint8_t*, unsigned int)> cb = std::bind(&ESPmqtt::_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  _mqtt->setCallback(cb);
}
bool ESPmqtt::loop() {
  if (!_mqtt->connected()) {
    this->connect();
    //_subscribe();
  }
  return _mqtt->loop();
}

void ESPmqtt::publish(const char *topic, const char *msg) {
  if ( topic != NULL && msg != NULL ) {
    String str = _mqtt_prfx + topic;
    _mqtt->publish(str.c_str(), msg);
  }
}

