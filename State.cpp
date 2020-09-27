#include "State.h"

namespace State {

  // para deep sleep
  //RTC_DATA_ATTR unsigned long time_mark=0;
  //RTC_DATA_ATTR float amb_temp_window[24*4];
  //RTC_DATA_ATTR float obj_temp_window[24*4];

  // para light sleep no es necesario el RTC_DATA_ATTR y en lighe si se conserve el millis !!
  RTC_DATA_ATTR int last_known_hour=-1;
  RTC_DATA_ATTR int last_known_minute=-1;
  RTC_DATA_ATTR int data_index=-1;
  RTC_DATA_ATTR unsigned long sleeping_delta=-1;
  
  
  RTC_DATA_ATTR int wake_cause=ESP_SLEEP_WAKEUP_EXT0; // true es que despertó por boton, false que fue el timer
  RTC_DATA_ATTR float amb_temp_window[24*4]={0};
  RTC_DATA_ATTR float obj_temp_window[24*4]={0};

  void setWakeupCause(int cause) {
    wake_cause=cause;
  }

  int getWakeupCause() {
    return wake_cause;
  }

  bool isReboot() {
    return last_known_hour==-1;
  }

  int getLastMinute() {
    return last_known_minute;
  }

  int getLastHour() {
    return last_known_hour;
  }
  
  void setData(int hour,int minute,float amb,float obj) {
    last_known_hour=hour;
    last_known_minute=minute;
    data_index=hour*4 + int(minute/15);
    amb_temp_window[data_index]=amb;
    obj_temp_window[data_index]=obj;
    for (int j=int(minute/15)+1; j<4; j++) {
      amb_temp_window[data_index+j]=0;
      obj_temp_window[data_index+j]=0;
    }
    Serial.printf("now: %02d:%02d  idx:%d\n",hour,minute,data_index);
    for (int i=0; i<24*4; i++) {
      Serial.printf("%5.2f   %5.2f\n",amb_temp_window[i],obj_temp_window[i]);
    }
  }

  bool initState() {
    // true setup for the first time
    Serial.println("This is a boot!!");
    Serial.printf("temp_window_size: %d\n",sizeof(amb_temp_window));
    memset(amb_temp_window,0,sizeof(amb_temp_window));
    memset(obj_temp_window,0,sizeof(obj_temp_window));
    return true;
  }
  
  
  void intoWindow(float lastAmbTmp,float lastObjTmp) {
    memcpy(amb_temp_window,amb_temp_window+1,sizeof(amb_temp_window)-sizeof(float));
    memcpy(obj_temp_window,obj_temp_window+1,sizeof(obj_temp_window)-sizeof(float));
    amb_temp_window[24*4-1]=lastAmbTmp;
    obj_temp_window[24*4-1]=lastObjTmp;
    for (int i=0; i<24*4; i++) {
      Serial.printf("%5.2f   %5.2f\n",amb_temp_window[i],obj_temp_window[i]);
    }
  }
  
  const float* getAmbData() {
    return amb_temp_window;
  }
  
  const float* getObjData() {
    return obj_temp_window;
  }

  void setSleepingDelta(unsigned long delta) {
    sleeping_delta=delta;
  }

  unsigned long getSleepingDelta() {
    return sleeping_delta;
  }
    
}
