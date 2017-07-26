#include "log.h"

String log::buf[10];
volatile uint8_t log::needWriting = 0;
volatile int log::index = 0;

void log::writeToFile() {
  if(log::needWriting) {
    File f = SPIFFS.open("/www/data/log.html", "a");
    for(int x = 0; x < index ; ++x) {
      char charBuf[log::buf[x].length()+1];
      log::buf[x].toCharArray(charBuf, log::buf[x].length() + 1);
      f.printf("<p>%u:%u:%u %u/%u/%u- %s</p>", hour(), minute(), second(), day(), month(), year(), charBuf);
    }
    log::index = 0;
    f.close();
    log::needWriting = false;
  }
}
void log::writeLog(String txt) {
  return;
  log::buf[index] = txt;
  if(log::index < 10) ++log::index;
  log::needWriting = true;
}
void log::deleteLog() {
  File f =SPIFFS.open("/www/data/log.html", "w");
  f.close();
}
