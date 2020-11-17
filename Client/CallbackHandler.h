#pragma once

#include <Communication.h>

class CallbackHandler : public CommCallbacks
{
public:
	//CallbackHandler();

	virtual void NewConnectionCB(const char *hostname);

	virtual void ConnectionLost();

	virtual void DataReceived(const char *data, unsigned len);

};

