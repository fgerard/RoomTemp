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

  Adafruit_MLX90614* temperature=NULL;

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

  float lastAmbTmp;
  float lastObjTmp;

  float getLastAmbTmp() {
    return lastAmbTmp;
  }
  
  float getLastObjTmp() {
    return lastObjTmp;
  }

  bool takeTemp() {
    lastObjTmp=temperature->readObjectTempC();
    lastAmbTmp=temperature->readAmbientTempC();
    return true;
  }

  bool setupTemperature() {
    xSemaphoreTakeRecursive(i2c_sem, portMAX_DELAY);
    temperature=new Adafruit_MLX90614();
    //temperature->begin(); // NO SE DA BEGIN PORQUE SOLO HACE Wire.begin y ya está hecho!!!
    
    Serial.println("Testing temperature");
    delay(500);
    float objT=temperature->readObjectTempC();
    float ambT=temperature->readAmbientTempC();
  
    Serial.println(format("Ambiente: %.1f, Object: %.1f",ambT,objT));
    //show(2000,"Temperature C",format("Ambient %.1f",ambT),format("Object %.1f",objT),"Rekonos...");
    xSemaphoreGiveRecursive( i2c_sem);
    return true;
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
  });
  
  btn_right.setPressedHandler([](Button2 & b) {
    btnRight++;
  });

  btn_left.setPressedHandler([](Button2 & b) {
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
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    rtc.adjust(DateTime(2020, 9, 24, 12, 0, 0));
  }
  
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  return true;
}

long currentTimeSecs() {
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

  void displayShow(int delta,String line1,String line2,String line3,String line4) {
    displayClear();
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    int lineHeight=tft.height()/4;
    tft.drawString(line1,0,lineHeight*0);
    tft.drawString(line2,0,lineHeight*1);
    tft.drawString(line3,0,lineHeight*2);
    tft.drawString(line4,0,lineHeight*3);
    delay(delta);
  }

  void displayPower(bool on) {
    tft.writecommand(on?TFT_DISPON:TFT_DISPOFF);
    digitalWrite(TFT_BL,on?HIGH:LOW);
    delay(200);
  }

  void displayGraph(int hour,int minute,const float* amb,const float* obj) {
    int data_index=hour*4 + int(minute/15);
    displayClear();
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(2);
    tft.drawString(format("%02d:%02d Tmp: %.1f-%.1f",hour,minute,amb[data_index],obj[data_index]),0,0);
    tft.setTextSize(1);
    uint16_t v = analogRead(ADC_PIN);
    float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
    //tft.drawString(format("Batery: %.2fV",battery_voltage),0,20);

    float minT,maxT,rangeT;
    minT = amb[data_index];
    maxT = amb[data_index];
    for (int n=0; n<24*4; n++) {
      if (amb[n]>0 && amb[n]<minT) minT=amb[n];
      if (amb[n]>maxT) maxT=amb[n];
      //if (obj[n]>0 && obj[n]<minT) minT=obj[n];
      //if (obj[n]>maxT) maxT=obj[n];
    }
    //minT -= 2;
    rangeT=maxT-minT;
    if (rangeT<1) rangeT=1;
    Serial.printf("min: %.1f, max: %.1f   range: %.1f\n",minT,maxT,rangeT);
    tft.drawString(format("Batery: %.2fV  min:%.1fC max:%.1fC",battery_voltage,minT,maxT,rangeT),0,20);

    int xOff=0;
    for (int n=0; n<24*4; n++) {
      xOff=int(n/4);
      int h=int(((amb[n]-minT)/rangeT)*60)+2;
      tft.drawFastVLine(n*2+xOff,tft.height()-h,h,obj[n]>amb[n]?TFT_BLUE:TFT_RED);
      tft.drawFastVLine(n*2+1+xOff,tft.height()-h,h,obj[n]>amb[n]?TFT_BLUE:TFT_RED);
      //h=int(((obj[n]-minT)/rangeT)*50)+2;
      //tft.drawFastVLine(n*2+1+xOff,tft.height()-h,h,obj[n]>amb[n]?TFT_RED:TFT_GREEN);
    }
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
        tft.drawString(format("Amb: %.1f%",Parts::getLastAmbTmp()),0,140);
        tft.setTextColor(Parts::getLastAmbTmp()>Parts::getLastObjTmp()?TFT_GREEN:TFT_RED,TFT_BLACK);
        tft.drawString(format("Obj: %.1f%",Parts::getLastObjTmp()),0,165);
      }
    }
  }


}
