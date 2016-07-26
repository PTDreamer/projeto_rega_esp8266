
#define ONLINE_UPDATE_PATH "/update"

#include <Arduino.h>
#include <Hash.h>
#include "fileHandling.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiSetup.h>
#include <ArduinoJson.h>
#include "ntp.h"
#include "outputs.h"
#include "ESPAsyncWebServer.h"
#include <ESP8266mDNS.h>
#include "ESPUpdateServer.h"
#include "HttpUpdateHandler.h"
#include <ArduinoOTA.h>
#include "log.h"
NTP ntp;
AsyncWebServer server(80);
fileHandling filehandler;
time_t getNtpTime() {
  return ntp.getNtpTime();
}
volatile bool schedulesUpdated = true;
void setupArduinoOTA() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    type = "sketch";
    else // U_SPIFFS
    type = "filesystem";
    SPIFFS.end();
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
}

void schedulesBodyHandler(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  if(request->method() == HTTP_POST && request->url() == "/data/schedules.json") {
    if(!index)
    Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject((const char*)data);
    if (!root.success())
    {
    //  Serial.println("parseObject() failed");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    if(!(root.containsKey("schedules") && root.containsKey("pump_delay"))) {
  //    Serial.println("Received json does not contain necessary keys");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    JsonArray& array = root["schedules"].asArray();
    if (!array.success())
    {
    //  Serial.println("schedules array failed");
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->print("{ \"status\": \"failure\"}");
      request->send(response);
      return;
    }
    File f = SPIFFS.open("/www/data/schedules.json", "w");
    f.print((const char*)data);
    f.close();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("{ \"status\": \"success\"}");
    request->send(response);
    schedulesUpdated = true;
    return;
  }
}
//online updating
HttpUpdateHandler httpUpdateHandler("","");
struct bootflags
{
    unsigned char raw_rst_cause : 4;
    unsigned char raw_bootdevice : 4;
    unsigned char raw_bootmode : 4;

    unsigned char rst_normal_boot : 1;
    unsigned char rst_reset_pin : 1;
    unsigned char rst_watchdog : 1;

    unsigned char bootdevice_ram : 1;
    unsigned char bootdevice_flash : 1;
};
struct bootflags bootmode_detect(void) {
    int reset_reason, bootmode;
    asm (
        "movi %0, 0x60000600\n\t"
        "movi %1, 0x60000200\n\t"
        "l32i %0, %0, 0x114\n\t"
        "l32i %1, %1, 0x118\n\t"
        : "+r" (reset_reason), "+r" (bootmode) /* Outputs */
        : /* Inputs (none) */
        : "memory" /* Clobbered */
    );

    struct bootflags flags;

    flags.raw_rst_cause = (reset_reason&0xF);
    flags.raw_bootdevice = ((bootmode>>0x10)&0x7);
    flags.raw_bootmode = ((bootmode>>0x1D)&0x7);

    flags.rst_normal_boot = flags.raw_rst_cause == 0x1;
    flags.rst_reset_pin = flags.raw_rst_cause == 0x2;
    flags.rst_watchdog = flags.raw_rst_cause == 0x4;

    flags.bootdevice_ram = flags.raw_bootdevice == 0x1;
    flags.bootdevice_flash = flags.raw_bootdevice == 0x3;

    return flags;
}

void setup() {
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(13, LOW);
  digitalWrite(14, LOW);
  digitalWrite(16, LOW);

  Serial.begin(115200);
  Serial.println("Start!");
  WiFiSetup::begin();
  uint8_t mdnssuccess = MDNS.begin("rega");
  ntp.setupNTP();
  setSyncProvider(getNtpTime);
  setSyncInterval(1 * 60);

  Serial.print("Connected to access point! IP Address=");
  Serial.print(WiFi.localIP());

  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
  } else {
    Serial.println("failed to mount FS");
  }
    //0:
  //1: normal boot
  //2: reset pin
  //3: software reset
  //4: watchdog reset - See more at: http://www.esp8266.com/viewtopic.php?p=2096#p2112
  bootflags flag = bootmode_detect();
  String reboot = String("REBOOT CAUSE:") + String(flag.raw_bootdevice);
  if(!mdnssuccess) {
    reboot = reboot + ("\nFailed to start mDNS");
  }
  log::writeLog(reboot);
  if(!SPIFFS.exists("/www/data/schedules.json")) {
    File f = SPIFFS.open("/www/data/schedules.json", "w");
    f.println("{\"schedules\": [],\"pump_delay\":0}");
    f.close();
  }
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  server.on("/data/getsystemstatus.json",[](AsyncWebServerRequest *request){
    FSInfo fs_info;
    SPIFFS.info(fs_info);
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
    json = String();
  });
  server.serveStatic("/fs", SPIFFS, "/");

  server.on("/data/schedules.json", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Request for schedules");
    request->send(SPIFFS, "/www/data/schedules.json");
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/www/index.html");
  });
  server.on("/data/schedules.json", HTTP_POST, [](AsyncWebServerRequest *request){
  }, NULL, schedulesBodyHandler);
  server.on("/data/gettime.json", [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->printf("{\"hour\" :%u,\"minute\" :%u,\"seconds\" :%u}", hour(), minute(), second());
    request->send(response);
  });
  server.on("/data/getoutputs.json", [](AsyncWebServerRequest *request){
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
      } else if(p->value() == "pump") {
        AsyncWebParameter *pp = request->getParam("state");
        if(pp->value() == "on")
          outputs::enableAuxPump();
        else if(pp->value() == "off")
          outputs::disableAuxPump();
      }
    }
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print(outputs::getOutputs());
    request->send(response);
  });

  server.serveStatic("/", SPIFFS, "/www/");

  server.on("/data/deletelog.html", [](AsyncWebServerRequest *request){
    log::deleteLog();
    request->send(200);
  });
  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.println("Request for not found");
    request->send(404);
  });
  httpUpdateHandler.setUrl(ONLINE_UPDATE_PATH);
  httpUpdateHandler.setVersion("0","0");
  server.addHandler(&httpUpdateHandler);
  server.begin();
  MDNS.addService("http", "tcp", 80);
  ESPUpdateServer_setup();

  setupArduinoOTA();
  ArduinoOTA.begin();
}

void loop() {
  log::writeToFile();
  ESPUpdateServer_loop();
  ArduinoOTA.handle();

  Alarm.delay(0);
  if(schedulesUpdated) {
    filehandler.handleScheduleFile();
    schedulesUpdated = false;
  }
}
