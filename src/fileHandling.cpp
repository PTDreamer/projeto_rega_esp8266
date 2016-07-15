#include "fileHandling.h"

bool fileHandling::handleScheduleFile() {
  if (!SPIFFS.exists("/shedules.json")) {
    Serial.println("shedules.json file doesnt exist");
    return false;
  }
  File schedulesFile = SPIFFS.open("/schedules.json", "r");
  if(!schedulesFile) {
    Serial.println("Could not open shedules.json for reading");
    return false;
  }
  Serial.println("opened shedules.json file");
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
  for(uint8_t i = 0; i < Alarm.count(); ++i) {
    Alarm.free(i);
  }
  outputs::setAuxPumpDelay(root["pump_delay"].as<int>());
  JsonArray& array = root["schedules"].asArray();
  for(JsonArray::iterator it=array.begin(); it!=array.end(); ++it)
  {
      JsonObject& value = it->as<JsonObject>();
      OnTick_t enable;
      OnTick_t disable;
      if(value["zone"] = "A") {
        enable = outputs::enableChannelA;
        disable = outputs::disableChannelA;
      } else {
        enable = outputs::enableChannelB;
        disable = outputs::disableChannelB;
      }
      time_t initial = value.get<int>("hour") * SECS_PER_HOUR + value.get<int>("minute") * SECS_PER_MIN;
      Alarm.alarmRepeat(initial, enable);
      Alarm.alarmRepeat(initial + value.get<int>("duration") * SECS_PER_MIN, disable);
  }
}
