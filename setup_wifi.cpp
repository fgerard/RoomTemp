#include "setup_wifi.h"


//https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//https://dl.espressif.com/dl/package_esp32_index.json

namespace SetupWifi {

  WebServer* server=NULL;
  bool eepromReady=false;
  String html;
  String content;
  int statusCode;

  void launchWeb();
  void setupAP(void);
  void createWebServer();

  String current_ssid;
  String current_pass;

  void(*show)(int,String,String,String,String);

  String readEEPROM(int offset,int len) {
    String tmp;
    for (int i=0; i<len; i++) {
      char c=char(EEPROM.read(i+offset));
      if (c==0) break;
      tmp+=c;
    }
    return tmp;
  }

  void clearEEPROM(int len) {
    for (int n=0; n<len; n++) {
      EEPROM.write(n,0);
    }
  }

  void writeEEPROM(int offset,int maxLen,String s) {
    for (int n=0; n<maxLen; n++) {
      EEPROM.write(offset+n,n<s.length()?s[n]:char(0));
    }
  }
  
  String getPersistedSSDI() {
    return readEEPROM(0,32);
  }  
  
  String getPersistedPass() {
    return readEEPROM(32,64);
  }

  String getBoxIP() {
    return readEEPROM(64,16);
  }

  bool setupWiFi(bool forceAP) {
    show=Parts::displayShow;
    Serial.println("setupWiFi... (1)");
    if (!eepromReady) EEPROM.begin(512);
    eepromReady=true;
    String ssdi=getPersistedSSDI();
    String pass=getPersistedPass();
    Serial.printf("ssdi: %s, pass: %s\n",ssdi.c_str(),pass.c_str());
    WiFi.begin(ssdi.c_str(),pass.c_str());
    bool ok=false;
    if (!forceAP) {
      for (int n=0; n<10; n++) {
         show(500,"WiFi","connecting to:",ssdi,format("try: %d",n+1));
        Serial.printf("ssdi: %s, pass: %s, try# %d\n",ssdi.c_str(),pass.c_str(),n);
        if (WiFi.status() == WL_CONNECTED) {
          ok=true;
          break;
        }
        delay(500);
      }
    }
    if (ok) {
      return true;
    }
    else {
      setupAP();
    }
    show(3000,"Connect to",format("'%s' WiFi",AP_NAME),"",format("http://%s",WiFi.softAPIP().toString().c_str()));
    while ((WiFi.status() != WL_CONNECTED)) {
      server->handleClient();
    }
  }
  
  void setupAP() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    show(500,"Scaning","networks","...","");
    int n = WiFi.scanNetworks();
    if (n == 0) {
      show(500,"No networks","found !","","");
    }
    else {
      show(1000,format("%d networks",n),"found !","","");
      for (int i = 0; i < n; ++i) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        show(1500,format("%d networks",n),format("%d",i+1),WiFi.SSID(i),"");
      }
    }
    Serial.println("");
    html = "<ol>";
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      html += "<li>";
      html += WiFi.SSID(i);
      html += " (";
      html += WiFi.RSSI(i);
  
      html += ")";
      //html += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
      html += "</li>";
    }
    html += "</ol>";
    delay(100);
    Serial.println(AP_NAME);
    Serial.println(AP_PASS);
    
    WiFi.softAP(AP_NAME, AP_PASS);
    launchWeb();
  }

  void launchWeb() {
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED)
      Serial.println("WiFi connected");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("SoftAP IP: ");
    Serial.println(WiFi.softAPIP());
    show(3000,"Connecto to","",WiFi.softAPIP().toString(),"");
    createWebServer();
    // Start the server
    server->begin();
    Serial.println("Server started");
  }

  void createWebServer() {
    if (!server) {
      current_ssid=getPersistedSSDI();
      current_pass=getPersistedPass();

      server=new WebServer(80);
    }

    server->on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += html;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32/><input name='pass' length=64/><br><label>Box ip:</label><input name='box' length=32/><br><label>Date:</label><input name='date' length=25/>><br><label>Time:</label><input name='time' length=25/><br><input type='submit'></form>";
      content += "</html>";
      server->send(200, "text/html", content);
    });
    server->on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server->send(200, "text/html", content);
    });

    server->on("/setting", []() {
      String Ddate = server->arg("date");
      Ddate.trim();
      String Dtime = server->arg("time");
      Dtime.trim();
      if (Ddate.length()>0 && Dtime.length()>0) {
        Serial.printf("Time adjusted to: %s %s\n",Ddate.c_str(),Dtime.c_str());
        Parts::adjust(Ddate.c_str(),Dtime.c_str());
      }
      else {
        Serial.println("Date or time not adjusted!!");
      }
      String ssid = server->arg("ssid");
      String pass = server->arg("pass");
      String boxIp = server->arg("box");
      if (ssid.length() > 0 && pass.length() > 0) {
        show(10000,"Persisting",ssid,pass,boxIp);
        Serial.printf("Persisting: %s, %s, %s\n",ssid.c_str(),pass.c_str(),boxIp.c_str());
        //clearEEPROM(64);
        writeEEPROM(0,32,ssid);
        writeEEPROM(32,64,pass);
        writeEEPROM(64,16,boxIp);
        EEPROM.commit();
        Serial.printf("Values written to EEPROM: '%s' '%s' '%s'\n",getPersistedSSDI().c_str(),getPersistedPass().c_str(),getBoxIP().c_str());
        content = "<html><h1>saved to eeprom... reboot into new wifi</h1></html>";
        statusCode = 200;
        show(3000,"Device","rebooting !!","","3s");
        
        //ESP.reset();
      } else {
        content = "<html><h1>404 Not found</h1></html>";;
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server->sendHeader("Access-Control-Allow-Origin", "*");
      server->send(statusCode, "text/html", content);
      Serial.println("reboot!!");
      delay(3000);
      Serial.println("bye");
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
      esp_sleep_enable_timer_wakeup(10000000);
      esp_deep_sleep_start();
      //esp_restart();
    });

    server->on("/box", []() {
      server->sendHeader("Access-Control-Allow-Origin", "*");
      server->send(100,"application/json",format("{'ssid':'%s','pass':'%s'}",current_ssid.c_str(),current_pass.c_str()));
    });

  }

}
