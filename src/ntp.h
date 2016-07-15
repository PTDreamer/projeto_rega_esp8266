#ifndef NTP_H
#define NTP_H

#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

class NTP {
public:
  NTP();
  void sendNTPpacket(IPAddress &address);
  time_t getNtpTime();
  void setupNTP();
private:
  WiFiUDP Udp;
  unsigned int localPort = 8888;  // local port to listen for UDP packets
  // NTP Servers:
  static const char ntpServerName[];
  //static const char ntpServerName[] = "time.nist.gov";
  //static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
  //static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
  //static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";
  const int timeZone = 1;     // Central European Time
  //const int timeZone = -5;  // Eastern Standard Time (USA)
  //const int timeZone = -4;  // Eastern Daylight Time (USA)
  //const int timeZone = -8;  // Pacific Standard Time (USA)
  //const int timeZone = -7;  // Pacific Daylight Time (USA)
  /*-------- NTP code ----------*/

  const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
  byte *packetBuffer;
};

#endif
