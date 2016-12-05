
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "WiFiSetup.h"



bool WiFiSetup::begin(char const *ssid,const char *passwd)
{
	WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
	#if SerialDebug != true
	wifiManager.setDebugOutput(false);
	#endif
    //reset saved settings
  //  wifiManager.resetSettings();

    //set custom ip for portal
    //and goes into a blocking loop awaiting configuration
    return wifiManager.autoConnect(ssid,passwd);
    //or use this for auto generated name ESP + ChipID
}
