// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CallbackHandler.h"
#include "Protocol.h"
#include "Interface.h"

#include <iostream>

#undef SHOWMESSAGE

using namespace std;

template <typename Enumeration>
auto as_integer(Enumeration const value)
-> typename std::underlying_type<Enumeration>::type
{
	return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

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
	std::shared_ptr<std::string> receiver = std::make_shared<std::string>(std::string(""));
	std::shared_ptr<CommCallbacks> callback(new CallbackHandler(receiver));
	Communication comm(callback);

	bool res = comm.Connect(servername, serverPort);

	std::vector<std::vector<std::shared_ptr<double>>*> channels = std::vector<std::vector<std::shared_ptr<double>>*>{};

	std::shared_ptr<TextBox> status = std::make_shared<TextBox>(TextBox(Binding(std::make_shared<std::string>(std::string("> connected to ") + servername), [](auto data) {
	}), std::make_optional<std::string>("log"), BorderSize::Double, 86, 6));


	std::shared_ptr<TextBox> input = std::make_shared<TextBox>(TextBox(Binding(std::make_shared<std::string>(std::string("")), [](auto data) {
	}), std::nullopt, BorderSize::Single, 86, 1));

	std::vector<std::shared_ptr<Widget>> rings = std::vector<std::shared_ptr<Widget>> {
		status,
		input
	};
	Column col = Column(rings, std::nullopt, BorderSize::None);

	Interface tui = Interface(std::shared_ptr<Column>(&col));

	if (!res) {
		cerr << "error connecting;" << endl;
	}


	bool configured = false;

	MessageFactory messageFactory = MessageFactory();


	for (;;) {
		tui.Draw();

		Reply* reply;

		std::string send;
		if (configured) {
			input.get()->Focus();

			std::string read = *(std::string*)input.get()->Binding()->Get().get();

			((std::string*)status.get()->Binding()->Get().get())->append("\n> " + read);

			if (read == "quit") {
				comm.Disconnect();
				return 0;
			}
			Command* command;

			command = messageFactory.Command(read);
			send = command->ToString();

			delete command;
		} else {
			send = Commands::Config().ToString();
		}
		comm.WriteToPartner(send.c_str(), send.length());
		
		while (!comm.IsMessagePending()) {
			__noop();
		}

		while (comm.IsMessagePending()) {
			comm.ProcessMessage();
			if (receiver.get()->empty()) {
				while (!comm.IsMessagePending()) {
					__noop();
				}
				continue;
			}
			reply = messageFactory.Reply(*receiver.get());
			((std::string*)status.get()->Binding()->Get().get())->append("\n> " + reply->ToString().substr(0, reply->ToString().length() - 2));
			switch (reply->StatusCode()) {
				case StatusCode::STATUS:
				case StatusCode::STATUS_DIFFERENCE:
				{
					std::vector<Value> values = ((Replies::StatusDifference*)reply)->GetStatus();

					for (auto& value : values) {
						*(channels[value.GetLight()]->at(as_integer(value.GetRing().value()) * 7 + as_integer(value.GetColor().value())).get()) = value.GetValue().value();
					}
					break;
				}
				case StatusCode::CONFIG:
					for (int i = 0; i < ((Replies::Config*)reply)->GetConfig(); i++) {
						std::vector<std::shared_ptr<double>>* channel = new std::vector<std::shared_ptr<double>>{};
						for (int j = 0; j < 7 * 4; j++) {
							channel->push_back(std::make_shared<double>(0));
						}
						channels.push_back(channel);

						rings.insert(rings.begin(), std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>>{
							std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(14), [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(15), [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(16), [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(17), [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(18), [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(19), [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(20), [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
						}, std::make_optional<std::string>("ring2"), BorderSize::Double)),

							std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(21), [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(22), [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(23), [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(24), [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(25), [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(26), [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(27), [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
							}, std::make_optional<std::string>("ring3"), BorderSize::Double)),
						}, std::nullopt, BorderSize::None)));

						rings.insert(rings.begin(), std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
							std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
								std::make_shared<Fader>(Fader(Binding(channels[i]->at(0), [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
									std::make_shared<Fader>(Fader(Binding(channels[i]->at(1), [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
									std::make_shared<Fader>(Fader(Binding(channels[i]->at(2), [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
									std::make_shared<Fader>(Fader(Binding(channels[i]->at(3), [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
									std::make_shared<Fader>(Fader(Binding(channels[i]->at(4), [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
									std::make_shared<Fader>(Fader(Binding(channels[i]->at(5), [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
									std::make_shared<Fader>(Fader(Binding(channels[i]->at(6), [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
							}, std::make_optional<std::string>("main"), BorderSize::Double)),

								std::make_shared<Row>(Row(std::vector<std::shared_ptr<Widget>> {
									std::make_shared<Fader>(Fader(Binding(channels[i]->at(7), [](auto data) {}), std::make_optional<std::string>("D"), BorderSize::None, 4, 6, 100)),
										std::make_shared<Fader>(Fader(Binding(channels[i]->at(8), [](auto data) {}), std::make_optional<std::string>("R"), BorderSize::Single, 4, 6, 100)),
										std::make_shared<Fader>(Fader(Binding(channels[i]->at(9), [](auto data) {}), std::make_optional<std::string>("G"), BorderSize::Single, 4, 6, 100)),
										std::make_shared<Fader>(Fader(Binding(channels[i]->at(10), [](auto data) {}), std::make_optional<std::string>("B"), BorderSize::Single, 4, 6, 100)),
										std::make_shared<Fader>(Fader(Binding(channels[i]->at(11), [](auto data) {}), std::make_optional<std::string>("W"), BorderSize::Single, 4, 6, 100)),
										std::make_shared<Fader>(Fader(Binding(channels[i]->at(12), [](auto data) {}), std::make_optional<std::string>("A"), BorderSize::Single, 4, 6, 100)),
										std::make_shared<Fader>(Fader(Binding(channels[i]->at(13), [](auto data) {}), std::make_optional<std::string>("U"), BorderSize::Single, 4, 6, 100)),
								}, std::make_optional<std::string>("ring1"), BorderSize::Double)),
						}, std::nullopt, BorderSize::None)));
					}
					configured = true;
					break;
			}
		}
		((std::string*)input.get()->Binding()->Get().get())->clear();
	}

	comm.Disconnect();

	return 0;
}



