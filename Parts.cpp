#include "Parts.h"

#define TFT_DISPOFF 0x10
#define TFT_DISPON   0x11

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL              4   // Display backlight control pin
#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_RIGHT        35
#define BUTTON_LEFT          0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn_right(BUTTON_RIGHT);
Button2 btn_left(BUTTON_LEFT);


namespace Parts {

  SemaphoreHandle_t i2c_sem = xSemaphoreCreateRecursiveMutex();

  //Adafruit_MLX90614* temperature=NULL;
  BlueDot_BME280 bme = BlueDot_BME280();

  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  RTC_DS3231 rtc;  

////////////////////////////////////// Serial ////////////////////////////////////
  bool setupSerial() {
    Serial.setDebugOutput(true);
    Serial.begin(115200);
    for (int i=0; i<5; i++) {
      Serial.println(".......");
    }
    Serial.println("Starting Rekono...");
    return true;
  }
  
////////////////////////////////////// I2C ////////////////////////////////////
  uint8_t setupI2C() {
    xSemaphoreTakeRecursive( i2c_sem, portMAX_DELAY);
    Wire.begin(SDA,SCL,I2C_speed);
    delay(300);
    uint8_t devCnt;
    for (int i=0; i<5; i++) {
      devCnt=scan_i2c();
      Serial.printf("i2c Devices found: %d\n",devCnt);
      if (devCnt==3) break;
      delay(300);
    }
    xSemaphoreGiveRecursive(i2c_sem);
    return devCnt;
  }


////////////////////////////////////// Temperature ////////////////////////////////////

  float lastTemp;
  float lastHumidity;
  float lastPressure;


  float getLastTemp() {
    return lastTemp;
  }
  
  float getLastPressure() {
    return lastPressure;
  }

  float getLastHumidity() {
    return lastHumidity;
  }

  bool takeTemp() {
    delay(250);
    lastTemp=bme.readTempC();
    lastHumidity=bme.readHumidity();
    lastPressure=bme.readPressure();
    Serial.printf("\ntakeTemp(1): %.1f   %.1f   %.1f\n",lastTemp,lastHumidity,lastPressure);
    delay(250);
    lastTemp=bme.readTempC();
    lastHumidity=bme.readHumidity();
    lastPressure=bme.readPressure();
    Serial.printf("takeTemp(2....): %.1f   %.1f   %.1f\n",lastTemp,lastHumidity,lastPressure);
    return true;
  }

  bool setupTemperature() {
    xSemaphoreTakeRecursive(i2c_sem, portMAX_DELAY);
    bme.parameter.communication = 0;
    bme.parameter.I2CAddress = 0x76;
    bme.parameter.sensorMode = 0b11;
    bme.parameter.IIRfilter = 0b100;
    bme.parameter.humidOversampling = 0b101; 
    bme.parameter.tempOversampling = 0b101;
    bme.parameter.pressOversampling = 0b101;
    bme.parameter.pressureSeaLevel = 1013.25; 
    bme.parameter.tempOutsideCelsius = 22; 
    int result=bme.init();
    Serial.printf("Result: %d\n",result);
    if (result != 0x58) {
      Serial.println("Problems initializing bme280...");
    }
    xSemaphoreGiveRecursive( i2c_sem);
    return result==0x60 || result==0x58;
  }
  
////////////////////////////////////// Gestures ////////////////////////////////////
  boolean setupGestures() {
    uint8_t error = paj7620Init();
    if (error) {
      //show(1000,"Gesture","Sensor","errors","");
    }
    else {
      //show(1000,"Gesture","sensor","ready!","");
    }
    return error?false:true; 
  }

  int confirmGesture(char* gestureName,int entryGesture) {
    uint8_t data;
    int result=entryGesture;
    delay(GES_ENTRY_TIME);
    paj7620ReadReg(0x43, 1, &data);
    if(data == GES_FORWARD_FLAG) {
      Serial.println("Forward");
      result=GES_FORWARD_FLAG;
    }
    else if(data == GES_BACKWARD_FLAG) {
      Serial.println("Backward");
      result=GES_BACKWARD_FLAG;
    }
    else {
      Serial.println(gestureName);
    }
    return result;
  }

  int getGesture() {
    uint8_t data=0,data1=0,error;
    error = paj7620ReadReg(0x43, 1, &data);       // Read Bank_0_Reg_0x43/0x44 for gesture result.
    if (!error) {
      int result=-1;
      
      switch (data) {
        case GES_RIGHT_FLAG:
          paj7620ReadReg(0x43, 1, &data);
          result=confirmGesture("Right",GES_RIGHT_FLAG);
          break;
        case GES_LEFT_FLAG: 
          paj7620ReadReg(0x43, 1, &data);
          result=confirmGesture("Left",GES_LEFT_FLAG);
          break;
        case GES_UP_FLAG:
          paj7620ReadReg(0x43, 1, &data);
          result=confirmGesture("Up",GES_UP_FLAG);
          break;
        case GES_DOWN_FLAG:
          paj7620ReadReg(0x43, 1, &data);
          result=confirmGesture("Down",GES_DOWN_FLAG);
          break;
        case GES_FORWARD_FLAG:
          Serial.println("Forward");
          result=GES_FORWARD_FLAG;
          break;
        case GES_BACKWARD_FLAG:     
          Serial.println("Backward");
          result=GES_BACKWARD_FLAG;
          break;
        case GES_CLOCKWISE_FLAG:
          Serial.println("Clockwise");
          result=GES_CLOCKWISE_FLAG;
          break;
        case GES_COUNT_CLOCKWISE_FLAG:
          Serial.println("counter-clockwise");
          result=GES_COUNT_CLOCKWISE_FLAG;
          break;  
        default:
          paj7620ReadReg(0x44, 1, &data1);
          if (data1 == GES_WAVE_FLAG) 
          {
            Serial.println("wave");
            result=GES_WAVE_FLAG;
          }
          break;
      }
      delay(GES_QUIT_TIME);
      return result;
    }
    else {
      return -1;
    }
  }

////////////////////////////////////// Buttons //////////////////////////////
int btnLeft=0;
int btnRight=0;

int buttonLeft() {
  return btnLeft;
}

void setButtonLeft() {
  btnLeft=1;
}

void resetButtonLeft() {
  btnLeft=0;
}

int buttonRight() {
  return btnRight;
}
void resetButtonRight() {
  btnRight=0;
}

void buttonInit() {
  btn_right.setLongClickHandler([](Button2 & b) {
    Serial.println("btn_right setLongClickHandler");
    delay(3000);
  });
  
  btn_right.setPressedHandler([](Button2 & b) {
    Serial.println("btn_right setPressedHandler");
    delay(3000);
    btnRight++;
  });

  btn_left.setPressedHandler([](Button2 & b) {
    State::changeSelectedGraph();
    State::setTimeMark();
    State::setWithDelay(true);
    btnLeft++;
  });
}

void buttonLoop() {
  btn_left.loop();
  btn_right.loop();
}

////////////////////////////////////// RTC //////////////////////////////////

bool RTCinit() {

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    return false;
  }
  Serial.println("RTCinit:");
  Serial.println(F(__DATE__));
  Serial.println(F(__TIME__));
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2020, 9, 24, 12, 0, 0));
  }
  
  DateTime now = rtc.now();
  
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.println(getDateAsStr());
  Serial.println();
  Serial.printf("unix time: %d\n",now.unixtime());
  return true;
}

void adjust(const char* date,const char* time) {
  rtc.adjust(DateTime(F(date), F(time)));
}

DateTime now() {
  return rtc.now();
}

String getDateAsStr() {
  DateTime now = rtc.now();
  String result=format("%04d-%02d-%02dT%02d:%02d:%02d",now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second());
  return result;
}

unsigned long currentTimeSecs() {
  DateTime now = rtc.now();
  return now.unixtime();
}

////////////////////////////////////// Parts ////////////////////////////////
  int vref = 1100;

  SemaphoreHandle_t initParts() {
    Serial.println("initParts....");
    
    //if (!setupSerial()) {
    //  Serial.println("no serial....");
    //  return NULL;
    //}
    Serial.println("****** RoomTemp starting... ********");

    /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);
  
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    if (TFT_BL > 0) {                           // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
        pinMode(TFT_BL, OUTPUT);                // Set backlight pin to output mode
        digitalWrite(TFT_BL, HIGH); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    }
     
    uint8_t devCnt=setupI2C();
    if (devCnt != 3) {
      Serial.printf("Product uses 1 devices, %d found, halting!\n",devCnt);
      return NULL;
    }
    if (!setupTemperature()) {
      Serial.println("Temperature sensor errors, halting!");
      //show(3000,"ERROR","Temperature","sensor","halting!!");
      return NULL;
    }
    buttonInit();

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV\n", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.println("Default Vref: 1100mV");
    }

    RTCinit();
    return i2c_sem;
  }

  void displayClear() {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
  }


  void displayShowNoClear(int delta,String line1,String line2,String line3,String line4) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    int lineHeight=tft.height()/4;
    tft.drawString(line1,0,lineHeight*0);
    tft.drawString(line2,0,lineHeight*1);
    tft.drawString(line3,0,lineHeight*2);
    tft.drawString(line4,0,lineHeight*3);
    delay(delta);
  }

  void displayShow(int delta,String line1,String line2,String line3,String line4) {
    displayClear();
    displayShowNoClear(delta, line1, line2, line3, line4);
  }



  void displayPower(bool on) {
    tft.writecommand(on?TFT_DISPON:TFT_DISPOFF);
    digitalWrite(TFT_BL,on?HIGH:LOW);
    delay(200);
  }

  void displayGraph(bool withDelay,DateTime current,const char* title,const char* fmat,const float* data) {
    int hour=current.hour();
    int minute=current.minute();
    int data_index=hour*4 + int(minute/15);
    //displayClear();
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString(title,0,0);    
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    
    uint16_t v = analogRead(ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    
    float minT,maxT,rangeT;
    minT = data[data_index];
    maxT = data[data_index];
    for (int n=0; n<24*4; n++) {
      if (data[n]>0 && data[n]<minT) minT=data[n];
      if (data[n]>maxT) maxT=data[n];
    }
    //minT -= 2;
    rangeT=maxT-minT;
    Serial.printf("min: %.1f, max: %.1f   range: %.1f\n",minT,maxT,rangeT);
    if (rangeT<1) rangeT=1;
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(format(fmat,data[data_index],minT,maxT),0,20);
    tft.setTextSize(1);
    tft.drawString(format("Batery: %.2fV",battery_voltage),0,40);
    //tft.setTextSize(2);
    int xOff=0;
    int n=data_index+1 % (24*4);
    for (int j=0; j<24*4; j++) {
      current=now();
      String zDate=format("%02d:%02d:%02d",current.hour(),current.minute(),current.second());
      tft.setTextSize(2);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString(zDate.c_str(),145,0);
      tft.setTextSize(1);
      xOff=int(n/4);
      int h=int(((data[n]-minT)/rangeT)*60)+2;
      tft.drawFastVLine(n*2+xOff,tft.height()-62,62,TFT_BLUE);
      tft.drawFastVLine(n*2+1+xOff,tft.height()-62,62,TFT_BLUE);
      if (withDelay) {
        delay(25);
      }
      if (n != data_index) {
        tft.drawFastVLine(n*2+xOff,tft.height()-62,62,TFT_BLACK);
        tft.drawFastVLine(n*2+1+xOff,tft.height()-62,62,TFT_BLACK);
      }
      tft.drawFastVLine(n*2+xOff,tft.height()-h,h,TFT_RED);
      tft.drawFastVLine(n*2+1+xOff,tft.height()-h,h,TFT_RED);
      tft.setTextColor(n==data_index?TFT_GREEN:TFT_RED, TFT_BLACK);
      tft.drawString(format("%3.1f",data[n]),150,40);
      if (withDelay) {
        delay(10);
      }
      n=++n % (24*4);
      //h=int(((obj[n]-minT)/rangeT)*50)+2;
      //tft.drawFastVLine(n*2+1+xOff,tft.height()-h,h,obj[n]>amb[n]?TFT_RED:TFT_GREEN);
    }
  }

  void displayData(DateTime current,const float* temp,const float* humidity,const float* pressure) {
    int hour=current.hour();
    int minute=current.minute();
    int data_index=hour*4 + int(minute/15);
    uint16_t v = analogRead(ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    float minT,maxT,rangeT;
    minT = temp[data_index];
    maxT = temp[data_index];
    for (int n=0; n<24*4; n++) {
      if (temp[n]>0 && temp[n]<minT) minT=temp[n];
      if (temp[n]>maxT) maxT=temp[n];
      //if (obj[n]>0 && obj[n]<minT) minT=obj[n];
      //if (obj[n]>maxT) maxT=obj[n];
    }
    //displayShowNoClear(100,Parts::getDateAsStr().c_str(),format("Batery: %.2fV  min:%.1fC max:%.1fC",battery_voltage,minT,maxT,rangeT),"Temp:",format("%.1f   %.1f",Parts::getLastAmbTmp(),Parts::getLastObjTmp()));
    displayShowNoClear(100,Parts::getDateAsStr().c_str(),format("Bat: %.2fV",battery_voltage),format("min:%.1fC max:%.1fC",minT,maxT),format("%.1f %.1f %.1f",getLastTemp(),getLastHumidity(),getLastPressure()));
  }


  void showVoltage() {
    static uint64_t timeStamp = 0;
    if (millis() - timeStamp > 1000) {
      timeStamp = millis();
      uint16_t v = analogRead(ADC_PIN);
      float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
      String voltage = "Voltage :" + String(battery_voltage) + "V";
      Serial.println(voltage);
      tft.fillScreen(TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.setTextColor(TFT_GREEN,TFT_BLACK);
      tft.setTextSize(1);
      tft.drawString(format("Voltage:%.2fV",battery_voltage),  tft.width() / 2, 10); //tft.height() / 2 );
      //tft.drawString(format("cnt: %d",++cnt),0,20);
      if (Parts::takeTemp()) {
        //tft.drawString(format("height:%d",tft.height()),5,25);
        //tft.drawString(format("width: %d",tft.width()),5,35);
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW,TFT_BLACK);
        tft.drawString(format("Temp: %.1f%",getLastTemp()),0,140);
        tft.setTextColor(TFT_RED,TFT_BLACK);
        tft.drawString(format("Pres: %.1f%",getLastPressure()),0,165);
      }
    }
  }


}
