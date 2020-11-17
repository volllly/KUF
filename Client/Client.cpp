// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CallbackHandler.h"
#include "Protocol.h"
#include "Interface.h"

#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

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

	std::vector<std::vector<std::shared_ptr<double>>> channels = std::vector<std::vector<std::shared_ptr<double>>>{
		std::vector<std::shared_ptr<double>> {
			std::make_shared<double>(0),
			std::make_shared<double>(0),
			std::make_shared<double>(100),
			std::make_shared<double>(50),
			std::make_shared<double>(30),
			std::make_shared<double>(4),
			std::make_shared<double>(25.5),

			std::make_shared<double>(0),
			std::make_shared<double>(0),
			std::make_shared<double>(100),
			std::make_shared<double>(50),
			std::make_shared<double>(30),
			std::make_shared<double>(4),
			std::make_shared<double>(25.5),

			std::make_shared<double>(0),
			std::make_shared<double>(0),
			std::make_shared<double>(100),
			std::make_shared<double>(50),
			std::make_shared<double>(30),
			std::make_shared<double>(4),
			std::make_shared<double>(25.5),

			std::make_shared<double>(0),
			std::make_shared<double>(0),
			std::make_shared<double>(100),
			std::make_shared<double>(50),
			std::make_shared<double>(30),
			std::make_shared<double>(4),
			std::make_shared<double>(25.5),
		}
	};


	std::shared_ptr<TextBox> status = std::make_shared<TextBox>(TextBox(Binding(std::make_shared<std::string>(std::string("> connected to ") + servername), [](auto data) {
	}), std::make_optional<std::string>("log"), BorderSize::Double, 86, 6));


	std::shared_ptr<TextBox> input = std::make_shared<TextBox>(TextBox(Binding(std::make_shared<std::string>(std::string("")), [](auto data) {
	}), std::nullopt, BorderSize::Single, 86, 1));

	Column col = Column(std::vector<std::shared_ptr<Widget>> {
		std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
			std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
				std::make_shared<Fader>(Fader(Binding(channels[0][0], [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][1], [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][2], [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][3], [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][4], [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][5], [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][6], [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
			}, std::make_optional<std::string>("main"), BorderSize::Double)),

			std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
				std::make_shared<Fader>(Fader(Binding(channels[0][7],  [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][8],  [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][9],  [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][10], [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][11], [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][12], [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][13], [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
			}, std::make_optional<std::string>("ring1"), BorderSize::Double)),
		}, std::nullopt, BorderSize::None)),
		std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
			std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
				std::make_shared<Fader>(Fader(Binding(channels[0][14], [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][15], [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][16], [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][17], [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][18], [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][19], [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][20], [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
			}, std::make_optional<std::string>("ring2"), BorderSize::Double)),

			std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
				std::make_shared<Fader>(Fader(Binding(channels[0][21], [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][22], [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][23], [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][24], [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][25], [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][26], [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
				std::make_shared<Fader>(Fader(Binding(channels[0][27], [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
			}, std::make_optional<std::string>("ring3"), BorderSize::Double)),
		}, std::nullopt, BorderSize::None)),
		status,
		input
	}, std::nullopt, BorderSize::None);

	Interface tui = Interface(std::shared_ptr<Column>(&col));

	for (;;) {

		tui.Draw();

		input.get()->Focus();

		std::string* read = (std::string*)input.get()->Binding()->Get().get();

		((std::string*)status.get()->Binding()->Get().get())->append("\n> " + *read);

		if (*read == "quit") {
			comm.Disconnect();
			return 0;
		}

		comm.WriteToPartner(read->c_str(), read->length() + 1);
		OS_Sleep(100);

		while (comm.IsMessagePending()) {
			comm.ProcessMessage();
			//((std::string*)status.get()->Binding()->Get().get())->append("\n> " + *receiver.get());
		}
		read->empty();
	}

	comm.Disconnect();

	return 0;
}



