#ifndef SETUP_WIFI_H
#define SETUP_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <WebServer.h>

#include "util.h"
#include "D_config.h"

namespace SetupWifi {

  String getPersistedSSDI();
  String getPersistedPass();
  String getBoxIP();

  bool setupWiFi(bool isReboot,void(*show_fn)(int,String,String,String,String),bool forceAP);

}

#endif
