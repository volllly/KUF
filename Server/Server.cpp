// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#define SHOWMESSAGE

#include "ServerCallbackHandler.h"


int main()
{
    std::cout << "Hello World!\n";

    unsigned short serverPort = 49191;

    ServerCallbackHandler *serverCallbackHandler = new ServerCallbackHandler();
    std::shared_ptr<CommCallbacks> myCallbackHandler(serverCallbackHandler);
    Communication myComm(myCallbackHandler);

    // link communication to callback handler
    serverCallbackHandler->SetCommunicationLayer(&myComm);

    myComm.Activate(serverPort);

    std::cout << "server started at port " << serverPort << std::endl;

    while (true) 
    {
      // wait some time - msecs
      OS_Sleep(100);

      // check for answers
      while (myComm.IsMessagePending())
        myComm.ProcessMessage();

    } 

    // theoretically
    myComm.Deactivate();

}

