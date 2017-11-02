///////////////////////////////////////
// ESP config class
///////////////////////////////////////

#include <ArduinoJson.h>

#ifndef ESPCONFIG_H
#define ESPCONFIG_H

//const char *ESPConfig_ConfigFileName PROGMEM = "/esp.cfg";

class ESPConfig {
  private:
    String _cfgFileName;
    DynamicJsonBuffer _jsonBuffer;
    bool _serviceMode = false;
    String _json;

    long _serviceFlagAddr = 0;
    long _serviceModeDealy = 100;
    bool _isServiceMode();
    void _setServiceMode();
    void _clearServiceMode();

    unsigned long _wifiTimeout;
    template <typename TStringRef, typename TValueRef>
    bool set_impl(TStringRef key, TValueRef value) {
      bool ret;
      JsonObject& params = _jsonBuffer.parseObject(_json);
      if ( params.success() ) {
        ret = params.set(key, value);
        _json = "";
        params.printTo(_json);
      } else {
        JsonObject& params = _jsonBuffer.createObject();
        ret = params.set(key, value);
        _json = "";
        params.printTo(_json);
      }
      return ret;
    }
  public:

    ESPConfig(const char *cfgFileName = "/esp.cfg");//ESPConfig_ConfigFileName);
    ~ESPConfig();

    bool loadCfg();
    bool saveCfg();
    String getCfg() {
      return _json;
    }
    void setCfg(char *cfg, size_t len = -1) {
      if ( len == -1 ) _json = cfg;
      else {
        char buf[len + 1];
        memcpy(buf, cfg, len);
        buf[len] = '\0';
        _json = buf;
      }
    }
    bool containsKey(const char *key);
    String get(const char *key);
    // Sets the specified key with the specified value.
    //
    // bool set(TKey, TValue);
    // TKey = const std::string&, const String&
    // TValue = bool, long, int, short, float, double, RawJson, JsonVariant,
    //          const std::string&, const String&,
    //          const JsonArray&, const JsonObject&
    template <typename TValue, typename TString>
    typename TypeTraits::EnableIf < !TypeTraits::IsArray<TString>::value & &
    !TypeTraits::IsArray<TValue>::value,
    bool >::type
    set(const TString& key, const TValue& value) {
      return set_impl<const TString&, const TValue&>(key, value);
    }
    //
    // bool set(TKey, TValue);
    // TKey = const std::string&, const String&
    // TValue = const char*, const char[N], const FlashStringHelper*
    template <typename TValue, typename TString>
    typename TypeTraits::EnableIf < !TypeTraits::IsArray<TString>::value,
             bool >::type
    set(const TString& key, const TValue* value) {
      return set_impl<const TString&, const TValue*>(key, value);
    }
    //
    // bool set(TKey, TValue);
    // TKey = const char*, const char[N], const FlashStringHelper*
    // TValue = bool, long, int, short, float, double, RawJson, JsonVariant,
    //          const std::string&, const String&,
    //          const JsonArray&, const JsonObject&
    template <typename TValue, typename TString>
    typename TypeTraits::EnableIf < !TypeTraits::IsArray<TValue>::value, bool >::type
    set(const TString* key, const TValue& value) {
      return set_impl<const TString*, const TValue&>(key, value);
    }
    //
    // bool set(TKey, TValue);
    // TKey = const char*, const char[N], const FlashStringHelper*
    // TValue = const char*, const char[N], const FlashStringHelper*
    template <typename TValue, typename TString>
    bool set(const TString* key, const TValue* value) {
      return set_impl<const TString*, const TValue*>(key, value);
    }
    //void set(const char *key, String& value);

    bool wait4ServiceMode(bool blync = true, int sdelay = 5);

    void begin(const char *ssid = NULL, const char *pass = NULL);
    //void handleClient();

    bool isServiceMode() {
      return _serviceMode;
    }
};

#endif
