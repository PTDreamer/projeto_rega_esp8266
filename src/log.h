#ifndef log_H
#define  log_H

#include <FS.h>
#include "TimeLib.h"

class log {
public:
  static void writeLog(String);
  static void deleteLog();
  static void writeToFile();
private:
  static String buf[100];
  static volatile int index;
  static volatile uint8_t needWriting;
};
#endif
