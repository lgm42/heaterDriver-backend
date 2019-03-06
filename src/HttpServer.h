// HttpServer.h

#ifndef _WEBSERVER_h
#define _WEBSERVER_h

#include "Arduino.h"

#include <ESP8266WebServer.h>
#include <NTPClientLib.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ESP8266FtpServer.h>
#include "DHT.h"
#include "IRManager.h"

class HttpServer
{
  public:
	  HttpServer(DHT & dht, IRManager & ir);
	  virtual ~HttpServer();

	virtual void setup(void);
	virtual void handle(void);

	String getContentType(String filename);

	ESP8266WebServer & webServer();
private:
	ESP8266WebServer _webServer;
	FtpServer _ftpServer;
	WiFiUDP _ntpUDP;
	NTPClient _timeClient;
	DHT & _dht;
	IRManager & _irManager;

	bool handleFileRead(String path);

	void sendOk();
	void sendKo(const String & message);
	void sendOkAnswerWithParams(const String & params);

	static const String DataKeyword;
	static const String RestKeyword;
	static const String LearnIrKeyword;
	static const String SendIrKeyword;

};

#endif

