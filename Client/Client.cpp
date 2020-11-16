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
		std::cout << Commands::Connect().ToString();
		std::cout << Commands::Version().ToString();
		std::cout << Commands::Set(std::vector{ 
			Value(1, Ring::MAIN, Color::RED, 66.6),
			Value(1, Ring::MAIN, Color::GREEN, 10),
			Value(1, Ring::MAIN, Color::BLUE, 23.5),
			}).ToString();
		std::cout << Commands::Reset(std::vector<unsigned int>{ 1, 2, 3, 4 }).ToString();
		std::cout << Commands::Status(std::vector<unsigned int>{ 1, 2, 3, 4 }).ToString();
		std::cout << Commands::Config().ToString();

		std::cout << endl;

		std::cout << Replies::Done().ToString();
		std::cout << Replies::Version("1").ToString();
		std::cout << Replies::Config(1).ToString();
		std::cout << Replies::StatusDifference(std::vector{
			Value(1, Ring::MAIN, Color::RED, 66.6),
			Value(1, Ring::MAIN, Color::GREEN, 10),
			Value(1, Ring::MAIN, Color::BLUE, 23.5),
			}).ToString();
		std::cout << Replies::Status(std::vector{
			Value(1, Ring::MAIN, Color::RED, 66.6),
			Value(1, Ring::MAIN, Color::GREEN, 10),
			Value(1, Ring::MAIN, Color::DIMM, 23.5),
			}).ToString();
		std::cout << Replies::NotFound(std::vector{
			Value(1, Ring::MAIN, Color::RED),
			Value(1, Ring::MAIN),
			Value(1),
			}).ToString();
		std::cout << Replies::Unknown("unknown: payload").ToString();
		std::cout << Replies::UnknownVersion(std::vector{ string("2"), string("3"), string("4") }).ToString();
		std::cout << Replies::Internal("some server error").ToString();


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



