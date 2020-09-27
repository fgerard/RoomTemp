#ifndef State_H
#define State_H

#include "util.h"
#include "D_config.h"

namespace State {
  void setData(int hour,int minute,float amb,float obj);
  void setWakeupCause(int cause);
  int getWakeupCause();
  bool isReboot();
  int getLastMinute();
  int getLastHour();
  
  bool initState();
  void intoWindow(float lastAmbTmp,float lastObjTmp);

  const float* getAmbData();
  const float* getObjData();

  void setSleepingDelta(unsigned long delta);
  unsigned long getSleepingDelta();

}

#endif
