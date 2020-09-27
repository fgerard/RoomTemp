#include "util.h"
#include "setup_wifi.h"
#include "Parts.h"
#include "State.h"

#define WINDOW_MINUTES 15

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP,"pool.ntp.org",-5*3600,15000);

void setup() {
  Serial.begin(115200);
  Serial.println("Setup running...");
  esp_sleep_wakeup_cause_t cause=esp_sleep_get_wakeup_cause();
  bool isReboot=State::isReboot();
  State::setWakeupCause(cause);

  if (!Parts::initParts()) {
    Serial.println("halting!");
    for(;;);
  }
  if (isReboot) Parts::displayShow(300,"Starting...","","","");

  for (int k=0; k<2; k++) {
    Serial.println("************************************************************");
  }
  Serial.printf("Is reboot: %d, cause: %d!=%d\n",isReboot, cause,ESP_SLEEP_WAKEUP_TIMER);
  int hour;
  int minute;
  if (isReboot || State::getWakeupCause()==ESP_SLEEP_WAKEUP_EXT0) {
    SetupWifi::setupWiFi(isReboot,Parts::displayShow,false);
    ntpClient.begin();
    ntpClient.update();
    ntpClient.forceUpdate();
    Parts::displayShow(980,"Hora actual:",ntpClient.getFormattedTime(),"","");
    hour=ntpClient.getHours();
    minute=ntpClient.getMinutes();
    Parts::takeTemp();
    State::setData(hour,minute,Parts::getLastAmbTmp(),Parts::getLastObjTmp());
  }
  else if (State::getWakeupCause()==ESP_SLEEP_WAKEUP_TIMER) {
    int hour=State::getLastHour();
    int minute=State::getLastMinute()+15;
    if (minute>=60) {
      minute=minute % 15;
      hour=(hour+1) % 24;
    }
    Serial.printf("Desperto, tomando datos: %02d:%02d\n",hour,minute);
    Parts::takeTemp();
    State::setData(hour,minute,Parts::getLastAmbTmp(),Parts::getLastObjTmp());
  }
}

void loop() {
  //ntpClient.update();
  Serial.printf("millis: %d\n",millis());
  unsigned long delta2next=15*60*1000000-millis()*1000; // 15 minutos menos 1seg
  if ((State::getWakeupCause()==ESP_SLEEP_WAKEUP_EXT0) || (State::getWakeupCause()==ESP_SLEEP_WAKEUP_EXT1)) {
    Parts::displayGraph(State::getLastHour(),State::getLastMinute(),State::getAmbData(),State::getObjData());
    delay(10000);
    if (State::getWakeupCause()==ESP_SLEEP_WAKEUP_EXT0) {
      delta2next=(15 - (State::getLastMinute() % 15))*60*1000000-millis()*1000; // lo que falta para 15 menos 10 del delay menos 1
    }
    else {
      delta2next = State::getSleepingDelta()-millis()*1000;
    }
  }
  Parts::buttonLoop();

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
  esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
  esp_sleep_enable_timer_wakeup(delta2next);
  State::setSleepingDelta(delta2next);
  Serial.printf("Bye!, see you in %d\n",delta2next/1000000);
  //  esp_light_sleep_start();
  esp_deep_sleep_start();
}
