// IRManager.h

#ifndef _IRMANAGER_H
#define _IRMANAGER_H

#include "Arduino.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>

#include "types.h"

class IRManager
{
  public:
	  IRManager();
	  virtual ~IRManager();

	virtual void setup(void);
	virtual void handle(void);

	void enableIRReceiver(const bool value);

	void startToLearnCode(int codeNumber);
	void sendIrCode(int codeNumber);

private:
	IRrecv _irReceiver;
	IrCode _lastDecodedFrame;
	int _learningCodeNumber;
	bool _codeHasBeenLearned;
	IRsend _irSender;

	int resultToRawData(const decode_results & results, IrCode & rawData, const int maxSize = 200);

	static const int IRReceiverPin = D1;
};

#endif

