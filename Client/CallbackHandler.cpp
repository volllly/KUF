#include "CallbackHandler.h"
#include <iostream>

CallbackHandler::CallbackHandler(std::shared_ptr<std::string> receiver) {
	_receiver = receiver;
}

void CallbackHandler::NewConnectionCB(const char *hostname)
{
  //std::cout << "got new connection from " << hostname << std::endl;
}

void CallbackHandler::ConnectionLost()
{
  //std::cout << "connection lost" << std::endl;
}

void CallbackHandler::DataReceived(const char *data, unsigned len)
{
	_receiver.get()->clear();
	_receiver.get()->append(data);
  // here we rely on the fact that the data is a string!
  //std::cout << "got new data >>" << data << "<< len " << len << std::endl;
}
