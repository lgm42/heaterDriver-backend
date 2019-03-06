// JsonConfiguration.h

#ifndef _JSONCONFIGURATION_h
#define _JSONCONFIGURATION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ArduinoJson.h>

#include "types.h"

#define IRCODE_MAX_NUMBER	10

class JsonConfiguration 
{
public:
	JsonConfiguration();
	virtual ~JsonConfiguration();

	virtual void setup();

	bool saveConfig();

	void restoreDefault();

	void setIrCode(const int IrCodeNumber, const IrCode & irCode);
	const IrCode & getIrCode(const int IrCodeNumber);
	String _hostname;
	String _ftpLogin;
	String _ftpPasswd;

	int _nextRemoteOrderAt;
	String _nextRemoteOrder;

	IrCode _irCodes[IRCODE_MAX_NUMBER];

	String toJson();
private:
	JsonObject& irCodesAsJson();

	StaticJsonBuffer<1000> _jsonBuffer;
};

#if !defined(NO_GLOBAL_INSTANCES)
extern JsonConfiguration Configuration;
#endif

#endif

