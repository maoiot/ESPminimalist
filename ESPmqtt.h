#include "ESPConfig.h"
#include <PubSubClient.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

//#define MQTT_MAX_PACKET_SIZE 256

#ifndef ESPMQTT_H
#define ESPMQTT_H

class ESPmqtt {
  private:
    WiFiClient *_clnt;
    PubSubClient *_mqtt;
    String _user;
    String _upwd;
    String _mqtt_prfx;
    void _callback(char* ptopic, byte* ppayload, unsigned int plength);
    void _subscribe();
  public:
    ESPmqtt(const char *prfx = NULL);
    ~ESPmqtt();

    void begin(ESPConfig *cfg);
    void begin(const char *server, const int port);
    bool connect();
    bool connect(const char *user, const char *upwd);

    bool loop();

    void publish(const char *topic, const char *msg);
};

#endif
