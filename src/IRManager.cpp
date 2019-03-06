// 
// 
// 
#include <FS.h>
#include <stdlib.h>
#include <stdint.h>

#include "JsonConfiguration.h"

#include "IRManager.h"

IRManager::IRManager()
	: _irReceiver(IRReceiverPin), _learningCodeNumber(-1), _codeHasBeenLearned(false), _irSender(D2, true)
{
}

IRManager::~IRManager()
{
}

void IRManager::setup(void)
{
    _irSender.begin();
}

void IRManager::startToLearnCode(int codeNumber)
{
	_irReceiver.enableIRIn(); 
	_learningCodeNumber = codeNumber;
	_codeHasBeenLearned = false;
}

void IRManager::sendIrCode(int codeNumber)
{
	IrCode irCode = Configuration.getIrCode(codeNumber);
	_irSender.sendRaw(irCode.data, irCode.size, 38);
	Serial.printf("Code %d sent\n", codeNumber);
}

void IRManager::handle(void)
{
	decode_results results;
  if ((not _codeHasBeenLearned) && (_learningCodeNumber >0) && (_irReceiver.decode(&results)))
	{
			// print() & println() can't handle printing long longs. (uint64_t)
		serialPrintUint64(results.value, HEX);
		Serial.println("");

		resultToRawData(results, _lastDecodedFrame);

		_irReceiver.disableIRIn();

		Configuration.setIrCode(_learningCodeNumber, _lastDecodedFrame);
		Configuration.saveConfig();

		_codeHasBeenLearned = true;
	}
}

int IRManager::resultToRawData(const decode_results & results, IrCode & rawData, const int maxSize) {

  if (rawData.data != NULL) 
	{
		free(rawData.data);
		rawData.data = NULL;
  }

  //check for memory 
  uint16_t expectedSize = getCorrectedRawLength(&results);
  if (expectedSize > maxSize)
		return -2;

  rawData.data = (uint16_t *)malloc(sizeof(uint16_t) * expectedSize);

  if (rawData.data == NULL)
	return -3;

  int index = 0;

  // Dump data
  for (uint16_t i = 1; i < results.rawlen; i++) {
    uint32_t usecs;
    for (usecs = results.rawbuf[i] * kRawTick; usecs > UINT16_MAX; usecs -= UINT16_MAX) {
      rawData.data[index++] = UINT16_MAX;
	  	rawData.data[index++] = 0;
    }
		rawData.data[index++] = usecs;
  }

  if (index != expectedSize)
	return -4;

	rawData.size = index;
  //everything ok
  return index;
}