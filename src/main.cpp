
#include <Arduino.h>
#include <Hash.h>
#include "fileHandling.h"

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <WiFiSetup.h>

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#include "ntp.h"
#include "outputs.h"

#include "ESPAsyncWebServer.h"
#include <ESP8266mDNS.h>
NTP ntp;
AsyncWebServer server(80);

time_t getNtpTime() {
  return ntp.getNtpTime();
}
void bodyHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  if(request->method() == HTTP_POST && request->url() == "/schedules.json") {
  if(!index)
    Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (!root.success())
    {
      Serial.println("parseObject() failed");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    if(!(root.containsKey("schedules") && root.containsKey("pump_delay"))) {
      Serial.println("Received json does not contain necessary keys");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    JsonArray& array = root["schedules"].asArray();
    if (!array.success())
    {
      Serial.println("schedules array failed");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    File f = SPIFFS.open("/schedules.json", "w");
    f.print((const char*)data);
    f.close();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"success\"}");
    request->send(response);
    return;
}
}
void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  WiFiSetup::begin();
  MDNS.begin("rega");
  ntp.setupNTP();
  setSyncProvider(getNtpTime);
  setSyncInterval(5 * 60);
  Serial.println(hour());
  Serial.println(minute());
  //1. Start WiFi

  Serial.print("Connected to access point! IP Address=");
  Serial.print(WiFi.localIP());
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  if(!SPIFFS.exists("/schedules.json")) {
    File f = SPIFFS.open("/schedules.json", "w");
    f.println("{\"schedules\": [],\"pump_delay\":0}");
    f.close();
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  server.on("/fsinfo",[](AsyncWebServerRequest *request){
  		FSInfo fs_info;
  		SPIFFS.info(fs_info);
  		request->send(200,"","totalBytes:" +String(fs_info.totalBytes) +
  		" usedBytes:" + String(fs_info.usedBytes)+" blockSize:" + String(fs_info.blockSize)
  		+" pageSize:" + String(fs_info.pageSize));
  		//testSPIFFS();
  });
  server.serveStatic("/fs", SPIFFS, "/");
  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.println("Request for not found");
		request->send(404);
  });
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
});
  server.on("/schedules.json", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Request for schedules");
    request->send(SPIFFS, "/schedules.json");
  });
  server.on("/schedules.json", HTTP_POST, [](AsyncWebServerRequest *request){
  }, NULL, bodyHandler);
  server.on("/gettime.html", [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->printf("{\"hour\" :%u,\"minute\" :%u,\"seconds\" :%u}", hour(), minute(), second());
    request->send(response);
  });
  server.begin();
   MDNS.addService("http", "tcp", 80);
}


void loop() {
  // put your main code here, to run repeatedly:
  Alarm.delay(1000); // wait one second between clock display
}
