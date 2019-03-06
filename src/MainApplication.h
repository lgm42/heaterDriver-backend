// MainApplication.h

#ifndef _MAINAPPLICATION_h
#define _MAINAPPLICATION_h

#include <ESP8266WiFi.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <Ticker.h>
#include <FS.h>
#include "DHT.h"

#include "HttpServer.h"
#include "JsonConfiguration.h"
#include "IRManager.h"

#include "Arduino.h"

class MainApplication
{
public:
	MainApplication();
	virtual ~MainApplication();

	virtual void setup(void);
	virtual void handle(void);
private:
	Ticker _ticker;
	WiFiManager _wifiManager;
	DHT _dht;
	IRManager _irManager;
	HttpServer _httpServer;

	bool runUpdate(Stream& in, uint32_t size, int command);

};

#endif

