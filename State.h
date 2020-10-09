#ifndef State_H
#define State_H

#include "init_values.h"
#include "util.h"
#include "D_config.h"

namespace State {
  void loadInitial();
  unsigned long setTimeMark();
  unsigned long getTimeMark();
  void setWithDelay(bool newVal);
  bool getWithDelay();
  
  void setData(int hour,int minute,float temp,float humidity,float pressure);
  void setWakeupCause(int cause);
  int getWakeupCause();
  bool isReboot();
  int getLastMinute();
  int getLastHour();
  
  bool initState();

  const float* getTempData();
  const float* getPressureData();
  const float* getHumidityData();

  int getSelectedGraph();
  void changeSelectedGraph();

  const float* getSelectedData();

  void setSleepingDelta(unsigned long delta);
  unsigned long getSleepingDelta();

}

#endif
