// 
// 
// 
#include <FS.h>
#include <ArduinoJson.h>

#include "JsonConfiguration.h"

JsonConfiguration::JsonConfiguration()
{
}

JsonConfiguration::~JsonConfiguration()
{
}

void JsonConfiguration::setup(void)
{
	File configFile = SPIFFS.open("/config.json", "r");
	if (!configFile) 
	{
		Serial.println("Failed to open config file");
		restoreDefault();
	}

	size_t size = configFile.size();
	if (size > 1024) 
	{
		Serial.println("Config file size is too large");
		restoreDefault();
	}
	
	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);

	StaticJsonBuffer<1000> jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) 
	{
		Serial.println("Failed to parse config file");
		restoreDefault();
	}
	
	_hostname = (const char*)json["hostname"];
	_ftpLogin = (const char*)json["ftp.login"];
	_ftpPasswd = (const char*)json["ftp.passwd"];

	_nextRemoteOrderAt = json["nextRemoteOrderAt"];
	_nextRemoteOrder = (const char*)json["nextRemoteOrder"];

	if ((_hostname.length() == 0) || (_ftpLogin.length() == 0)
		|| (_ftpPasswd.length() == 0))
	{
		Serial.println("Invalid configuration values, restoring default values");
		restoreDefault();
	}

	Serial.printf("hostname : %s\n", _hostname.c_str());
	Serial.printf("ftpLogin : %s\n", _ftpLogin.c_str());
	Serial.printf("ftpPasswd : %s\n", _ftpPasswd.c_str());
	Serial.printf("nextRemoteOrderAt : %d\n", _nextRemoteOrderAt);
	Serial.printf("_nextRemoteOrder : %s\n", _nextRemoteOrder.c_str());
}

bool JsonConfiguration::saveConfig()
{
	StaticJsonBuffer<1000> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["hostname"] = _hostname;
	json["ftp.login"] = _ftpLogin;
	json["ftp.passwd"] = _ftpPasswd;
	json["nextRemoteOrderAt"] = _nextRemoteOrderAt;
	json["nextRemoteOrder"] = _nextRemoteOrder;			 
	
	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) 
	{
		Serial.println("Failed to open config file for writing");
		return false;
	}

	json.printTo(configFile);
	
	return true;
}

void JsonConfiguration::restoreDefault()
{
	_hostname = "heater";
	_ftpLogin = "heater";
	_ftpPasswd = "heater";
	_nextRemoteOrderAt = 0;
	_nextRemoteOrder = "";

	saveConfig();
	Serial.println("configuration restored.");
}

String JsonConfiguration::toJson()
{
	String response = "{\"hostname\":\"" + Configuration._hostname + "\", 					\
	\"ftp-login\":\"" + Configuration._ftpLogin + "\",					\
	\"ftp-passwd\":\"" + Configuration._ftpPasswd + "\",					\
	\"next-remote-order-at\":\"" + Configuration._nextRemoteOrderAt + "\",					\
	\"next-remote-order\":\"" + Configuration._nextRemoteOrder + "\"}";
	return response;
}

#if !defined(NO_GLOBAL_INSTANCES) 
JsonConfiguration Configuration;
#endif