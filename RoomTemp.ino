#include "util.h"
#include "setup_wifi.h"
#include "Parts.h"
#include "State.h"

#define WINDOW_MINUTES 15

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP,"pool.ntp.org",-5*3600,15000);

esp_sleep_wakeup_cause_t cause;
DateTime current;
unsigned long lastMillis;
int n;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup running...");
  cause=esp_sleep_get_wakeup_cause();
  bool isReboot=State::isReboot();
  State::setWakeupCause(cause);
  Serial.printf("wakeup cause: %d    %d, %d, %d, %d\n",cause,ESP_SLEEP_WAKEUP_UNDEFINED,ESP_SLEEP_WAKEUP_EXT0, ESP_SLEEP_WAKEUP_EXT1,ESP_SLEEP_WAKEUP_TIMER );

  if (!Parts::initParts()) {
    Serial.println("halting!");
    for(;;);
  }
  if (isReboot) Parts::displayShow(300,"Starting...","","","");

  for (int k=0; k<2; k++) {
    Serial.println("************************************************************");
  }
  Serial.printf("Is reboot: %d, cause: %d!=%d\n",isReboot, cause,ESP_SLEEP_WAKEUP_TIMER);
  Serial.printf("wakeup cause: %d    %d, %d, %d, %d\n",cause,ESP_SLEEP_WAKEUP_UNDEFINED,ESP_SLEEP_WAKEUP_EXT0, ESP_SLEEP_WAKEUP_EXT1,ESP_SLEEP_WAKEUP_TIMER );

  int hour;
  int minute;
  if (cause==ESP_SLEEP_WAKEUP_UNDEFINED) {
    SetupWifi::setupWiFi(true);
    /*
    ntpClient.begin();
    ntpClient.update();
    ntpClient.forceUpdate();
    hour=ntpClient.getHours();
    minute=ntpClient.getMinutes();
    Parts::takeTemp();
    State::setData(hour,minute,Parts::getLastAmbTmp(),Parts::getLastObjTmp());
    Parts::displayShow(3000,"Hora actual:",ntpClient.getFormattedTime(),"Temp:",format("%.1f   %.1f",Parts::getLastAmbTmp(),Parts::getLastObjTmp()));
    */
  }
  current = Parts::now();
  Serial.printf("Desperto, tomando datos: %sn",Parts::getDateAsStr().c_str());
  Parts::takeTemp();
  State::setData(current.hour(),current.minute(),Parts::getLastAmbTmp(),Parts::getLastObjTmp());
  lastMillis=millis(); 
  n=0;
}

void go2sleep() {
  current = Parts::now();
  Serial.printf("unix: %d, 15min:%d, delta %d\n",current.unixtime(),(15 * 60),(16*60)-current.unixtime() % (15 * 60));
  unsigned long nextWakeUp = (15*60)-Parts::currentTimeSecs() % (15 * 60);
  Serial.printf("Current time: %s  delta:%0.1f\n",Parts::getDateAsStr().c_str(),nextWakeUp/60.0);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
  esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
  esp_sleep_enable_timer_wakeup(nextWakeUp * 1000000);
  State::setSleepingDelta(nextWakeUp);
  Serial.printf("Bye!, see you in %d secs\n",nextWakeUp);
  //  esp_light_sleep_start();
  esp_deep_sleep_start();  
}



void loop() {
  current = Parts::now();
  if (cause==ESP_SLEEP_WAKEUP_EXT1) {
    if ((millis()-lastMillis)>10000) {
      go2sleep();
    }
    Parts::takeTemp();
    Parts::displayShowNoClear(1,Parts::getDateAsStr().c_str(),"","Temp:",format("%.1f   %.1f",Parts::getLastAmbTmp(),Parts::getLastObjTmp()));
    Serial.printf("... %d\n",++n);
  }
  else {
    if (cause==ESP_SLEEP_WAKEUP_EXT0) {
      Parts::displayGraph(current.hour(),current.minute(),State::getAmbData(),State::getObjData());
      delay(10000);
    }
    go2sleep();
  }
  Parts::buttonLoop();
}
