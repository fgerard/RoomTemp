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
  RTC_DATA_ATTR float temp_window[24*4]={0};
  RTC_DATA_ATTR float humidity_window[24*4]={0};
  RTC_DATA_ATTR float pressure_window[24*4]={0};

  RTC_DATA_ATTR int selectedGraph=0;

  RTC_DATA_ATTR bool load_initial_flg=true;

  float initial_data_tmp[24*4] = {
    INIT_DATA_TMP
  };

  float initial_data_hummidity[24*4] = {
    INIT_DATA_HUMIDITY
  };

  float initial_data_pressure[24*4] = {
    INIT_DATA_PRESSURE
  };


  void loadInitial() {
    if (load_initial_flg) {
      for (int i=0; i<24*4; i++) {
        temp_window[i]=initial_data_tmp[i];
        humidity_window[i]=initial_data_hummidity[i];
        pressure_window[i]=initial_data_pressure[i];
      }
    }
    load_initial_flg=false;
  }

  unsigned long timeMark=0;
  bool withDelay=false;

  unsigned long setTimeMark() {
    timeMark=millis();
    return timeMark;
  }
  
  unsigned long getTimeMark() {
    return timeMark;
  }

  void setWithDelay(bool newVal) {
    withDelay=newVal;
  }
  
  bool getWithDelay() {
    return withDelay;
  }

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
  
  void setData(int hour,int minute,float temp,float humidity,float pressure) {
    Serial.printf("setData: %.1f %.1f %.1f\n",temp,humidity,pressure);
    last_known_hour=hour;
    last_known_minute=minute;
    data_index=hour*4 + int(minute/15);
    temp_window[data_index]=temp;
    humidity_window[data_index]=humidity;
    pressure_window[data_index]=pressure;
    for (int j=int(minute/15)+1; j<4; j++) {
      temp_window[data_index+j]=0;
      humidity_window[data_index+j]=0;
      pressure_window[data_index+j]=0;
    }
    Serial.printf("now: %02d:%02d  idx:%d\n",hour,minute,data_index);
    for (int i=0; i<24*4; i++) {
      Serial.printf("%5.2f   %5.2f  %5.2f\n",temp_window[i],humidity_window[i],pressure_window[i]);
    }
  }

  bool initState() {
    // true setup for the first time
    Serial.println("This is a boot!!");
    Serial.printf("temp_window_size: %d\n",sizeof(temp_window));
    memset(temp_window,0,sizeof(temp_window));
    memset(humidity_window,0,sizeof(humidity_window));
    memset(pressure_window,0,sizeof(pressure_window));
    return true;
  }
  
    
  const float* getTempData() {
    return temp_window;
  }
  
  const float* getHumidityData() {
    return humidity_window;
  }

  const float* getPressureData() {
    return pressure_window;
  }

  void setSleepingDelta(unsigned long delta) {
    sleeping_delta=delta;
  }

  unsigned long getSleepingDelta() {
    return sleeping_delta;
  }

  int getSelectedGraph() {
    return selectedGraph;
  }
  
  void changeSelectedGraph() {
    selectedGraph=++selectedGraph % 3;
  }

  const float* getSelectedData() {
    return selectedGraph==0?temp_window:selectedGraph==1?humidity_window:pressure_window;
  }
    
}
