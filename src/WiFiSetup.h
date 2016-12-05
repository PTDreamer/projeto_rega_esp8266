#ifndef WiFiSetup_H
#define WiFiSetup_H
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

class WiFiSetup
{
public:
	static bool begin(char const *ssid,const char *passwd=NULL);
};

#endif
