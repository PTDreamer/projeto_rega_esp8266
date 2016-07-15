#ifndef fileHandling_H
#define  fileHandling_H

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ArduinoJson.h>
#include <Time.h>
#include "outputs.h"

class fileHandling {
public:
  bool handleScheduleFile();
};
#endif
