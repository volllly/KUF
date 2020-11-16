// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CallbackHandler.h"
#include "Protocol.h"

#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	unsigned short serverPort = 8000;
	const char* servername = nullptr;

	// expect client to connect to as argument
	// if no arg start as server

	if (argc > 1)
	{
		servername = argv[1];
	}
	else
	{
#ifdef _DEBUG
		servername = "localhost";
#else
		cerr << "need server name to connect to" << endl;
		return -1;
#endif // DEBUG

	}

	std::shared_ptr<CommCallbacks> callback(new CallbackHandler());
	Communication comm(callback);

	bool res = comm.Connect(servername, serverPort);

	if (res)
	{
		MessageFactory messageFactory = MessageFactory();
		std::cout << messageFactory.Reply("200")->ToString();
		std::cout << messageFactory.Reply("210: 1")->ToString();
		std::cout << messageFactory.Reply("211: 3")->ToString();
		std::cout << messageFactory.Reply("202: 1 main red 10.6; 1 main green 1.6")->ToString();
		std::cout << messageFactory.Reply("203: 1 main blue 10.6; 1 main uv 1.6;")->ToString();
		std::cout << messageFactory.Reply("300: 3")->ToString();
		std::cout << messageFactory.Reply("300: 3 main")->ToString();
		std::cout << messageFactory.Reply("300: 3 main red")->ToString();
		std::cout << messageFactory.Reply("400: unknown command")->ToString();
		std::cout << messageFactory.Reply("401: 1; 2; 3;")->ToString();
		std::cout << messageFactory.Reply("500: error message")->ToString();

		std::cout << messageFactory.Command("connect: 1; 2; 3;")->ToString();
		std::cout << messageFactory.Command("version")->ToString();
		std::cout << messageFactory.Command("status: 1; 3")->ToString();
		std::cout << messageFactory.Command("status: 1")->ToString();
		std::cout << messageFactory.Command("set: 1 main red 10.6; 1 main green 1.6")->ToString();
		std::cout << messageFactory.Command("set: 1 main blue 10.6; 1 main uv 1.6;")->ToString();
		std::cout << messageFactory.Command("reset: 3")->ToString();
		std::cout << messageFactory.Command("reset: 3; 2")->ToString();
		std::cout << messageFactory.Command("config")->ToString();

		char inputstr[100];

		do
		{
			memset(inputstr, 0, sizeof(inputstr));
			std::cin.getline(inputstr, sizeof(inputstr));

			std::cout << "sending >>" << inputstr << std::endl;

			// send command
			comm.WriteToPartner(inputstr, strlen(inputstr) + 1);

			// wait some time - msecs
			OS_Sleep(100);

			// check for answers
			while (comm.IsMessagePending())
				comm.ProcessMessage();

		}
		while (strcmp(inputstr, "ENDE"));
	}
	else
	{
		cerr << "could not connect to server" << endl;
	}
	comm.Disconnect();

	return 0;
}



