#include "outputs.h"
#include "log.h"

bool outputs::outputsChanged = false;

void outputs::enableChannelA() {
  Serial.println("channel A enable");
  digitalWrite(13, HIGH);   // sets the LED on
  log::writeLog("Zone A activated");
  outputsChanged = true;
}
void outputs::disableChannelA() {
  Serial.println("disable channel A");
  digitalWrite(13, LOW);
  log::writeLog("Zone A deactivated");
  outputsChanged = true;
}
void outputs::enableChannelB() {
  Serial.println("channel B enable");
  digitalWrite(14, HIGH);
  log::writeLog("Zone B activated");
  outputsChanged = true;
}
void outputs::disableChannelB() {
  Serial.println("disable channel B");
  digitalWrite(14, LOW);
  log::writeLog("Zone B deactivated");
  outputsChanged = true;
}
void outputs::enableAuxPump() {
  Serial.println("pump enable");
  digitalWrite(12, HIGH);
  outputsChanged = true;
}
void outputs::disableAuxPump() {
  Serial.println("pump disable");
  digitalWrite(12, LOW);
  outputsChanged = true;

}

String outputs::getOutputs() {
  String za;
  String zb;
  String zp;
  if(digitalRead(13) == HIGH) {
    za = "on";
  } else {
    za = "off";
  }
  if(digitalRead(14) == HIGH) {
    zb = "on";
  } else {
    zb = "off";
  }
  if(digitalRead(12) == HIGH) {
    zp = "on";
  } else {
    zp = "off";
  }
  return "{\"zoneA\":\"" + za + "\",\"zoneB\":\"" + zb + "\",\"pump\":\"" + zp + "\"}";
}
