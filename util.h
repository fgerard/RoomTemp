#ifndef UTIL_H
#define UTIL_H
#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "RTClib.h"

void blink(int n,int delta_high,int delta_low);

String format(const char* fmt,...);

uint8_t scan_i2c();


#endif
