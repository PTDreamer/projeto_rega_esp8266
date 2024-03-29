#include <Arduino.h>
#include <Hash.h>
#include "fileHandling.h"
#include "log.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "outputs.h"
#include "ESPAsyncWebServer.h"
#include <WiFiSetup.h>
#include <ESP8266mDNS.h>
// #include <AsyncElegantOTA.h>
#include <ArduinoOTA.h>
#include "time.h"
#include <cstdlib>
//#include <TimeLib.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600);


#define NTP_MIN_VALID_EPOCH 1533081600

#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"   


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
uint8_t dd[2000];
bool led = false;
unsigned long led_last_time = 0;
fileHandling filehandler;
time_t getNtpTime()
{
  return time(NULL);
}
volatile bool schedulesUpdated = true;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    outputs::outputsChanged = true;
    Serial.println("socket connected");
  }
}
void schedulesBodyHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  size_t len2 = len;
  log::writeLog("len " + String(len) + " index:" + String(index) + " total:" + String(total));
  if ((index + len) > sizeof(dd))
  {
    return;
  }
  memcpy(dd + (index), data, len);
  if ((index + len) != total)
  {
    return;
  }

  dd[total] = 0;
  if (request->method() == HTTP_POST && request->url() == "/data/schedules.json")
  {
    if (!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char *)data);

    DynamicJsonDocument root(1501);
    DeserializationError error = deserializeJson(root, (const char *)dd);

    if (error)
    {
      //  Serial.println("parseObject() failed");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\", \"code\": \"" + String(error.c_str()) + "\", \"string\": \"" + String((const char *)dd) + "\", \"Lenght\": \"" + String(len2) + "\", \"index\": \"" + String(index) + "\", \"Total\": \"" + String(total) + "\"}");
      // response->print(error.c_str());
      request->send(response);
      return;
    }
    if (!(root.containsKey("schedules")))
    {
      //    Serial.println("Received json does not contain necessary keys");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\" , \"code\": \"no schedules key\"}");
      request->send(response);
      return;
    }

    File f = LittleFS.open("/www/data/schedules.json", "w");
    f.print((const char *)dd);
    f.close();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"success\"}");
    request->send(response);
    schedulesUpdated = true;
    return;
  }
}

void setup()
{
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(13, LOW);
  digitalWrite(14, LOW);
  digitalWrite(12, LOW);
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
  digitalWrite(15, LOW);
  digitalWrite(16, LOW);

  Serial.begin(115200);
  Serial.println("Start!");
  WiFi.hostname("rega2");
  const char *ssid = "NossaCasa";          // replace with your SSID
  const char *password = "renatacarolina"; // replace with your password
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // if(!WiFiSetup::begin("RegaConfig")) {
  // Serial.println("failed to connect and hit timeout");
  // ESP.reset();
  // delay(1000);
  //}

  Serial.println("mounting FS...");

  if (LittleFS.begin())
  {
    Serial.println("mounted file system");
  }
  else
  {
    Serial.println("failed to mount FS");
  }
  log::writeLog(ESP.getResetReason());
  log::writeLog(ESP.getResetInfo());

  if (!LittleFS.exists("/www/data/schedules.json"))
  {
    File f = LittleFS.open("/www/data/schedules.json", "w");
    f.println("{\"schedules\": []}");
    f.close();
  }
  server.on("/data/getsystemstatus.json", [](AsyncWebServerRequest *request)
            {
    FSInfo fs_info;
    LittleFS.info(fs_info);
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"analog\":"+String(analogRead(A0));
    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += ", \"fstotalbytes\":"+String(fs_info.totalBytes);
    json += ", \"fsusedbytes\":"+String(fs_info.usedBytes);
    json += ", \"fsblocksize\":"+String(fs_info.blockSize);
    json += ", \"fspagesize\":"+String(fs_info.pageSize);
    json += "}";
    request->send(200, "text/json", json);
    json = String(); });
  server.serveStatic("/fs", LittleFS, "/");

  server.on("/data/schedules.json", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("Request for schedules");
    request->send(LittleFS, "/www/data/schedules.json"); });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/www/index.html"); });
  server.on(
      "/data/schedules.json", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, schedulesBodyHandler);
  server.on("/data/gettime.json", [](AsyncWebServerRequest *request)
            {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->printf("{\"hour\" :%u,\"minute\" :%u,\"seconds\" :%u}", hour(), minute(), second());
    request->send(response); });
  server.on("/data/getoutputs.json", [](AsyncWebServerRequest *request)
            {
    if(request->hasParam("zone")) {
      AsyncWebParameter* p = request->getParam("zone");
      if(p->value() == "A") {
        AsyncWebParameter *pp = request->getParam("state");
        if(pp->value() == "on")
          outputs::enableChannelA();
        else if(pp->value() == "off")
          outputs::disableChannelA();
      } else if(p->value() == "B") {
        AsyncWebParameter *pp = request->getParam("state");
        if(pp->value() == "on")
          outputs::enableChannelB();
        else if(pp->value() == "off")
          outputs::disableChannelB();
      } else if(p->value() == "C") {
        AsyncWebParameter *pp = request->getParam("state");
        if(pp->value() == "on")
          outputs::enableChannelC();
        else if(pp->value() == "off")
          outputs::disableChannelC();
      } else if(p->value() == "D") {
        AsyncWebParameter *pp = request->getParam("state");
        if(pp->value() == "on")
          outputs::enableChannelD();
        else if(pp->value() == "off")
          outputs::disableChannelD();
      } else if(p->value() == "X") {
        AsyncWebParameter *pp = request->getParam("state");
        if(pp->value() == "on")
          outputs::enableAuxExit();
        else if(pp->value() == "off")
          outputs::disableAuxExit();
      }
    }
    request->send(200); });

  server.serveStatic("/", LittleFS, "/www/");

  server.on("/data/deletelog.html", [](AsyncWebServerRequest *request)
            {
    log::deleteLog();
    request->send(200); });
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    Serial.println("Request for not found");
    request->send(404); });
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.begin();
  timeClient.begin();

  //setSyncProvider(getNtpTime);
  //setSyncInterval(10);
  //configTime(MY_TZ, "us.pool.ntp.org");

  time_t tnow;


  ArduinoOTA.onStart([]()
  {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else {
        type = "filesystem";
        LittleFS.end();
      }
    });
  ArduinoOTA.begin();
}
void loop()
{
  timeClient.update();
   setTime(timeClient.getEpochTime());
  ArduinoOTA.handle();
  log::writeToFile();

  Alarm.delay(10);
  if (schedulesUpdated && (timeStatus() != timeNotSet))
  {
    log::writeLog(String(timeStatus()));
    filehandler.handleScheduleFile();
    schedulesUpdated = false;
  }
  unsigned long time_now = millis();
  if (time_now - led_last_time > 1000)
  {
    led_last_time = time_now;
    led ^= true;
    if (led)
      digitalWrite(5, LOW);
    else
      digitalWrite(5, HIGH);
    Serial.println(timeStatus());
    String s = "TIME|{\"hour\" :" + String(hour()) + ",\"minute\" :" + String(minute()) + ",\"seconds\" :" + String(second()) + "}";
    ws.textAll(s);
  }
  if (outputs::outputsChanged)
  {
    ws.textAll(String("OUTPUTS|") + outputs::getOutputs());
    outputs::outputsChanged = false;
  }
}
