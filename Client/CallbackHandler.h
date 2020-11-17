#pragma once

#include <Communication.h>

class CallbackHandler : public CommCallbacks
{
private:
	std::shared_ptr<std::string> _receiver;

public:
	CallbackHandler(std::shared_ptr<std::string> receiver);

	virtual void NewConnectionCB(const char *hostname);

	virtual void ConnectionLost();

	virtual void DataReceived(const char *data, unsigned len);

};

