#include "fileHandling.h"
#include "log.h"
bool fileHandling::handleScheduleFile() {
  if (!SPIFFS.exists("/www/data/schedules.json")) {
    Serial.println("schedules.json file doesnt exist");
    return false;
  }
  File schedulesFile = SPIFFS.open("/www/data/schedules.json", "r");
  if(!schedulesFile) {
    Serial.println("Could not open schedules.json for reading");
    return false;
  }
  Serial.println("opened schedules.json file");
  size_t size = schedulesFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  schedulesFile.readBytes(buf.get(), size);
  
  DynamicJsonDocument root(1024);
  DeserializationError error = deserializeJson(root, buf.get());
  
  if (!error) {
    Serial.println("\nparsed json");
  } else {
    Serial.println("failed to load json config");
    return false;
  }
  schedulesFile.close();
  uint8_t currentAlarms = Alarm.count();
  for(uint8_t i = 0; i < currentAlarms; ++i) {
    Alarm.free(i);
  }
  JsonArray array = root["schedules"].as<JsonArray>();
  int number = 0;
  String ids;
  for(JsonArray::iterator it=array.begin(); it!=array.end(); ++it)
  {
    JsonObject value = it->as<JsonObject>();
    OnTick_t enable;
    OnTick_t disable;
    if(value["zone"].as<String>() == "A") {
      Serial.println("ZONE A");
      enable = outputs::enableChannelA;
      disable = outputs::disableChannelA;
    } else if(value["zone"].as<String>() == "B") {
      Serial.println("ZONE B");
      enable = outputs::enableChannelB;
      disable = outputs::disableChannelB;
    } else if(value["zone"].as<String>() == "C") {
      Serial.println("ZONE C");
      enable = outputs::enableChannelC;
      disable = outputs::disableChannelC;
    } else if(value["zone"].as<String>() == "D") {
      Serial.println("ZONE D");
      enable = outputs::enableChannelD;
      disable = outputs::disableChannelD;
    } else if(value["zone"].as<String>() == "E") {
      Serial.println("ZONE E");
      enable = outputs::enableChannelE;
      disable = outputs::disableChannelE;
    } else if(value["zone"].as<String>() == "X") {
      Serial.println("aux out");
      enable = outputs::enableAuxExit;
      disable = outputs::disableAuxExit;
    }
    time_t initial = value["hour"].as<int>() * SECS_PER_HOUR + value["minute"].as<int>() * SECS_PER_MIN;
    Serial.println(initial);
    Serial.println(now());
    int id;
    id = Alarm.alarmRepeat(initial, enable);
    log::writeLog("Initial " + String(initial) + "id:" + id);
    ids = ids + String(id) + String(", ");
    id = Alarm.alarmRepeat(initial + value["dur"].as<int>() * SECS_PER_MIN, disable);
    ids = ids + String(id) + String(", ");
    ++number;
  }
  Serial.println(String("Loaded:") + String(number) + String(" schedules ") + ids);
  log::writeLog(String("Loaded:") + String(number) + String(" schedules ") + ids);
}
