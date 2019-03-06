// 
// 
// 
#include <FS.h>

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

	JsonObject& json = _jsonBuffer.parseObject(buf.get());

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

	//before parsing ircode we make correct initialization of the array
	for (int i = 0; i < IRCODE_MAX_NUMBER; ++i)
	{
		_irCodes[i].size = 0;
		_irCodes[i].data = NULL;
	}

	JsonObject& ircodes = json["irCodes"];

	//obj.get<int>("low");
	for (JsonPair& irCode : ircodes) {
		int index = atoi(irCode.key);
		JsonArray & array = irCode.value;
		
		_irCodes[index].data = (uint16_t *)malloc(sizeof(uint16_t) * array.size());
		 
		for (int j = 0; j < (int)array.size(); ++j) {
			_irCodes[index].data[j] = array[j].as<int>();
		}
		_irCodes[index].size = array.size();
	}

	Serial.printf("hostname : %s\n", _hostname.c_str());
	Serial.printf("ftpLogin : %s\n", _ftpLogin.c_str());
	Serial.printf("ftpPasswd : %s\n", _ftpPasswd.c_str());
	Serial.printf("nextRemoteOrderAt : %d\n", _nextRemoteOrderAt);
	Serial.printf("_nextRemoteOrder : %s\n", _nextRemoteOrder.c_str());
	String irCodesAsString;
	//irCodesAsJson().printTo(irCodesAsString);
	Serial.printf("irCodes : %s\n", irCodesAsString.c_str());

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

	//ir codes
	JsonObject& irCodes = json.createNestedObject("irCodes");

	for (int i = 0; i < IRCODE_MAX_NUMBER; ++i)
	{
		Serial.printf("Managing irCode[%d\n", i);
		JsonArray& array = irCodes.createNestedArray(String(i));
		for (int j = 0; j < _irCodes[i].size; ++j)
		{
			Serial.printf("add(%d\n", i);
			array.add(_irCodes[i].data[j]);
		}
		String tmp;
		array.printTo(tmp);
		Serial.println(tmp);
	}

	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) 
	{
		Serial.println("Failed to open config file for writing");
		return false;
	}

	json.printTo(configFile);

	String res;
	json.printTo(res);
	Serial.println(res.c_str());

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

String uint16ToString(uint16_t input, uint8_t base) {
   String result = "";
   do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c += '0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

void JsonConfiguration::setIrCode(const int irCodeNumber, const IrCode & irCode)
{
	if (_irCodes[irCodeNumber].data != NULL)
	{
		free(_irCodes[irCodeNumber].data);
		_irCodes[irCodeNumber].data = NULL;
		_irCodes[irCodeNumber].size = 0;
	}

	_irCodes[irCodeNumber].data = (uint16_t *)malloc(sizeof(uint16_t) * irCode.size);
	memcpy(_irCodes[irCodeNumber].data, irCode.data, irCode.size * sizeof(uint16_t));
	_irCodes[irCodeNumber].size = irCode.size;

	Serial.printf("_irCodes[%d].size = %d", irCodeNumber, _irCodes[irCodeNumber].size);
	Serial.println();

	for (int i = 0; i < _irCodes[irCodeNumber].size; ++i)
		Serial.printf("%d, ", _irCodes[irCodeNumber].data[i]);
	Serial.println();
}

const IrCode & JsonConfiguration::getIrCode(const int irCodeNumber)
{
	return _irCodes[irCodeNumber];
}

// JsonObject& JsonConfiguration::irCodesAsJson()
// {
// 	JsonObject& irCodes = _jsonBuffer.createObject();

// 	for (int i = 0; i < IRCODE_MAX_NUMBER; ++i)
// 	{
// 		Serial.printf("Managing irCode[%d\n", i);
// 		JsonArray& array = irCodes.createNestedArray("irCodes");
// 		for (int j = 0; j < _irCodes[i].size; ++j)
// 		{
// 			Serial.printf("add(%d\n", i);
// 			array.add(_irCodes[i].data[j]);
// 		}
// 		String tmp;
// 		array.printTo(tmp);
// 		Serial.println(tmp);
// 		irCodes[String(i)] = array;
// 	}

// 	String res;
// 	irCodes.printTo(res);
// 	Serial.println(res.c_str());
// 	return irCodes;
// }

// String JsonConfiguration::toJson()
// {
// 	String response = "{\"hostname\":\"" + Configuration._hostname + "\", 					\
// 	\"ftp-login\":\"" + Configuration._ftpLogin + "\",					\
// 	\"ftp-passwd\":\"" + Configuration._ftpPasswd + "\",					\
// 	\"next-remote-order-at\":\"" + Configuration._nextRemoteOrderAt + "\",					\
// 	\"next-remote-order\":\"" + Configuration._nextRemoteOrder + "\",						\
// 	\"irCodes\":\"" + irCodesAsJson() + "\"}";
// 	Serial.println(response);
// 	return response;
// }

#if !defined(NO_GLOBAL_INSTANCES) 
JsonConfiguration Configuration;
#endif