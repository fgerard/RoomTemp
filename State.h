#ifndef State_H
#define State_H

#include "util.h"
#include "D_config.h"

namespace State {
  void setData(int hour,int minute,float tmp,float humidity,float pressure);
  void setWakeupCause(int cause);
  int getWakeupCause();
  bool isReboot();
  int getLastMinute();
  int getLastHour();
  
  bool initState();

  const float* getTempData();
  const float* getPressureData();
  const float* getHumidityData();
  

  void setSleepingDelta(unsigned long delta);
  unsigned long getSleepingDelta();

}

#endif
