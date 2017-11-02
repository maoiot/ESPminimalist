#include "ESPConfig.h"
#include <FS.h>
#include <EEPROM.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

ESPConfig::ESPConfig(const char *cfgFileName) {
  _cfgFileName = String(cfgFileName);
  _json = "";
  if ( !SPIFFS.begin() ) SPIFFS.format();
}
ESPConfig::~ESPConfig() {
  SPIFFS.end();
}
bool ESPConfig::loadCfg() {
  if ( SPIFFS.begin() ) {
    File configFile = SPIFFS.open(_cfgFileName, "r");
    if (!configFile) return false;

    size_t size = configFile.size();
    if (size > 1024) return false;

    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    JsonObject& params = _jsonBuffer.parseObject(buf.get());
    configFile.close();

    if (!params.success()) {
      SPIFFS.remove(_cfgFileName);
      return false;
    }

    _json = "";
    params.printTo(_json);

    return true;
  } else return false;
}

bool ESPConfig::saveCfg() {
  if ( SPIFFS.begin() ) {
    File configFile = SPIFFS.open(_cfgFileName, "w");
    if (!configFile) return false;

    JsonObject& params = _jsonBuffer.parseObject(_json);
    params.printTo(configFile);
    configFile.close();

    return true;
  } else return false;
}
bool ESPConfig::containsKey(const char *key) {
  JsonObject& params = _jsonBuffer.parseObject(_json);
  return params.containsKey(key);
}
String ESPConfig::get(const char *key) {
  JsonObject& params = _jsonBuffer.parseObject(_json);

  if ( params.containsKey(key) )
    return params.get<String>(key);
  else
    return String("");
}
bool ESPConfig::_isServiceMode() {
  EEPROM.begin(1);
  char val = EEPROM.read(_serviceFlagAddr);
  delay(100);
  EEPROM.end();
  if (val != 'X') return false;
  else return true;
}
void ESPConfig::_setServiceMode() {
  EEPROM.begin(1);
  EEPROM.write(_serviceFlagAddr, 'X');
  EEPROM.commit();
  delay(100);
  EEPROM.end();
}
void ESPConfig::_clearServiceMode() {
  EEPROM.begin(1);
  EEPROM.write(_serviceFlagAddr, 0);
  EEPROM.commit();
  delay(100);
  EEPROM.end();
}
bool ESPConfig::wait4ServiceMode(bool blync, int sdelay) {
  bool ret;
  if (blync) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if ( _isServiceMode() ) {
    ret = true;
  } else {
    _setServiceMode();
    for (int i = sdelay; i >= 0; i--) {
      if (blync)
        digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      if (blync)
        digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
    }
    ret = false;
  }
  _clearServiceMode();
  return ret;
}

const char *keyHostname   = "hostname";
const char *keySSID       = "ssid";
const char *keyPass       = "pass";

const char *ESPConfig_apPass PROGMEM = "therisnospoon";

void ESPConfig::begin(const char *ssid, const char *pass) {
  uint8_t i;

  if (!wait4ServiceMode()) {
    _serviceMode = false;
    if ( loadCfg() ) {
      WiFi.mode(WIFI_STA);
      if ( containsKey(keyHostname) ) {
        WiFi.hostname(this->get(keyHostname));
      }
      if ( ssid == NULL ) {
        if ( containsKey(keySSID) && containsKey(keyPass) ) {
          WiFi.begin(this->get(keySSID).c_str(), this->get(keyPass).c_str());
        } else {
          WiFi.mode(WIFI_AP);
        }
      } else
        WiFi.begin(ssid, pass);
    } else if ( ssid != NULL )
      WiFi.begin(ssid, pass);
    if ( WiFi.getMode() == WIFI_STA )
      if ( WiFi.waitForConnectResult() != WL_CONNECTED )
        WiFi.mode(WIFI_AP);
  } else {
    WiFi.mode(WIFI_AP);
    _serviceMode = true;
  }
  if ( WiFi.getMode() == WIFI_AP ) {
    WiFi.softAP(WiFi.hostname().c_str(), ESPConfig_apPass);
  }
  if ( !_serviceMode ) {
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
  }
}


