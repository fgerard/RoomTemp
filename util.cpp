#include "util.h"

void blink(int n,int delta_high,int delta_low) {
#ifdef FLASH
  for (int i=0; i<n; i++) {
    digitalWrite(FLASH,HIGH);
    vTaskDelay(delta_high/portTICK_PERIOD_MS);
    digitalWrite(FLASH,LOW);
    vTaskDelay(delta_low/portTICK_PERIOD_MS);
  }
#endif
}

#define FMT_LINE_SIZE 256
char fmt_line[FMT_LINE_SIZE];

String format(const char* fmt,...) {
  va_list args;
  va_start(args,fmt);
  int n = vsnprintf(fmt_line,FMT_LINE_SIZE,fmt,args);
  if ((n<0) || (n>=FMT_LINE_SIZE)) {
    strcpy(fmt_line,"FMT error"); 
  }
  return fmt_line;
}

uint8_t scan_i2c() {
  byte error, address;
  uint8_t nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  return nDevices;
}

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms) {
  esp_sleep_enable_timer_wakeup(ms * 1000);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_light_sleep_start();
}
