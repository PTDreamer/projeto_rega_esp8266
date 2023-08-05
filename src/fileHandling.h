#ifndef fileHandling_H
#define  fileHandling_H

#include <LittleFS.h>
#include <ArduinoJson.h>
#include "outputs.h"

class fileHandling {
public:
  bool handleScheduleFile();
};
#endif
