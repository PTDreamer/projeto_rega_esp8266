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
void outputs::enableChannelC() {
  Serial.println("channel C enable");
  digitalWrite(15, HIGH);
  log::writeLog("Zone C activated");
  outputsChanged = true;
}
void outputs::disableChannelC() {
  Serial.println("disable channel C");
  digitalWrite(15, LOW);
  log::writeLog("Zone C deactivated");
  outputsChanged = true;
}
void outputs::enableChannelD() {
  Serial.println("channel D enable");
  digitalWrite(4, HIGH);
  log::writeLog("Zone D activated");
  outputsChanged = true;
}
void outputs::disableChannelD() {
  Serial.println("disable channel D");
  digitalWrite(4, LOW);
  log::writeLog("Zone D deactivated");
  outputsChanged = true;
}
void outputs::enableAuxExit() {
  Serial.println("aux enable");
  digitalWrite(12, HIGH);
  outputsChanged = true;
}
void outputs::disableAuxExit() {
  Serial.println("aux disable");
  digitalWrite(12, LOW);
  outputsChanged = true;

}

String outputs::getOutputs() {
  String za;
  String zb;
  String zc;
  String zd;
  String zx;
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
  if(digitalRead(15) == HIGH) {
    zc = "on";
  } else {
    zc = "off";
  }
  if(digitalRead(4) == HIGH) {
    zd = "on";
  } else {
    zd = "off";
  }
  if(digitalRead(12) == HIGH) {
    zx = "on";
  } else {
    zx = "off";
  }
  return "{\"zoneA\":\"" + za + "\",\"zoneB\":\"" + zb + "\",\"zoneC\":\"" + zc + "\",\"zoneD\":\"" + zd + "\",\"aux\":\"" + zx + "\"}";
}
