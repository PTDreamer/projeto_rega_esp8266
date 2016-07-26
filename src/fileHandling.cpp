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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(buf.get());
  root.printTo(Serial);
  if (root.success()) {
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
  int PumpDelay(root["pump_delay"].as<int>());
  JsonArray& array = root["schedules"].asArray();
  int number = 0;
  String ids;
  for(JsonArray::iterator it=array.begin(); it!=array.end(); ++it)
  {
    JsonObject& value = it->as<JsonObject>();
    OnTick_t enable;
    OnTick_t disable;
    if(value.get<String>("zone") == "A") {
      Serial.println("ZONE A");
      enable = outputs::enableChannelA;
      disable = outputs::disableChannelA;
    } else {
      enable = outputs::enableChannelB;
      disable = outputs::disableChannelB;
    }
    time_t initial = value.get<int>("hour") * SECS_PER_HOUR + value.get<int>("minute") * SECS_PER_MIN;
    Serial.println(initial);
    Serial.println(now());
    int id;
    id = Alarm.alarmRepeat(initial + PumpDelay, outputs::enableAuxPump);
    ids = ids + String(id) + String(", ");
    id = Alarm.alarmRepeat(initial, enable);
    ids = ids + String(id) + String(", ");
    id = Alarm.alarmRepeat(initial + value.get<int>("duration") * SECS_PER_MIN, outputs::disableAuxPump);
    ids = ids + String(id) + String(", ");
    id = Alarm.alarmRepeat(initial + value.get<int>("duration") * SECS_PER_MIN + PumpDelay, disable);
    ids = ids + String(id) + String(", ");
    ++number;
  }
  log::writeLog(String("Loaded:") + String(number) + String(" schedules ") + ids);
}
