#ifndef Parts_H
#define Parts_H

#include <Arduino.h>
#include <Wire.h>

#include <TFT_eSPI.h>
#include <FS.h>

#include <SPI.h>
#include "WiFi.h"
#include <Button2.h>
#include "esp_adc_cal.h"

#include "util.h"

//#include <Adafruit_MLX90614.h>
#include "BlueDot_BME280.h"
#include "paj7620.h"

#include "D_config.h"

namespace Parts {

  bool RTCinit();

  void buttonInit();
  void buttonLoop();
  
  int buttonLeft();
  void setButtonLeft();
  void resetButtonLeft();
  
  int buttonRight();
  void resetButtonRight();

  // inicializa los sensores 'partes' del Rekoni y regresa el semaforo que sincroniza el acceso al bus i2c
  SemaphoreHandle_t initParts();

  float getLastTemp();
  float getLastHumidity();
  float getLastPressure();
  bool takeTemp();

  void displayShow(int delta,String line1,String line2,String line3,String line4);
  void displayShowNoClear(int delta,String line1,String line2,String line3,String line4);
  void displayClear();
  void displayPower(bool on);
  void displayGraph(DateTime current,const float* temp,const float* humidity,const float* pressure);
  void displayData(DateTime current,const float* temp,const float* humidity,const float* pressure);

  DateTime now();
  String getDateAsStr();
  unsigned long currentTimeSecs();
  void adjust(const char* date,const char* time);


  //int getGesture();
  
}

#endif
