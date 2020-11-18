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

	std::shared_ptr<TextBox> status = std::make_shared<TextBox>(TextBox(std::make_shared<std::string>(std::string("")), std::make_shared<std::string>("log"), BorderSize::Double, 88, 6));


	std::shared_ptr<TextBox> input = std::make_shared<TextBox>(TextBox(std::make_shared<std::string>(std::string("")), nullptr, BorderSize::Single, 88, 1));

	std::shared_ptr<std::vector<std::shared_ptr<Widget>>> rings = std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {
		status,
		input
	});
	Column col = Column(rings, nullptr, BorderSize::None);

	Interface tui = Interface(std::shared_ptr<Column>(&col));

	if (!res) {
		cerr << "error connecting;" << endl;
	}

	enum class State {
		Connect,
		Configure,
		Idle
	};
	State state = State::Connect;

	MessageFactory messageFactory = MessageFactory();


	for (;;) {
		tui.Draw();

		Reply* reply;

		std::string send;
		switch(state) {
		case State::Idle:
		{
			input.get()->Focus();

			std::string read = *(std::string*)input.get()->Text().get();

			((std::string*)status.get()->Text().get())->append("\n> " + read);

			Command* command;

			command = messageFactory.Command(read);
			send = command->ToString();

			delete command;
			break;
		}
		case State::Configure:
			send = Commands::Config().ToString();

			((std::string*)status.get()->Text().get())->append("\n> " + send.substr(0, send.length() - 2));
			break;
		case State::Connect:
			send = Commands::Connect().ToString();

			((std::string*)status.get()->Text().get())->append("\n> " + send.substr(0, send.length() - 2));
			break;
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
			((std::string*)status.get()->Text().get())->append("\n< " + reply->ToString().substr(0, reply->ToString().length() - 2));
			switch (reply->StatusCode()) {
			case StatusCode::VERSION: 
				if (((Replies::Version*)reply)->GetVersion() != "1") {
					return 1;
				}
				if (state == State::Connect) {
					state = State::Configure;
				}
				break;
			case StatusCode::STATUS:
			case StatusCode::STATUS_DIFFERENCE:
			{
				std::vector<Value> values = ((Replies::StatusDifference*)reply)->GetStatus();

				for (auto& value : values) {
					*(channels[value.GetLight() - 1]->at(as_integer(value.GetRing().value()) * 7 + as_integer(value.GetColor().value())).get()) = value.GetValue().value();
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
				}

				const std::string names[] = { "D", "R", "G", "B", "W", "A", "U" };

				for (int i = ((Replies::Config*)reply)->GetConfig() - 1; i >= 0; i--) {
					auto light = std::make_shared<Column>(Column(std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {
					}), std::make_shared<std::string>("ring3"), BorderSize::Double));

					std::shared_ptr<std::vector<std::shared_ptr<Widget>>> main = std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {});
					std::shared_ptr<std::vector<std::shared_ptr<Widget>>> ring1 = std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {});
					std::shared_ptr<std::vector<std::shared_ptr<Widget>>> ring2 = std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {});
					std::shared_ptr<std::vector<std::shared_ptr<Widget>>> ring3 = std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {});

					for (int j = 0; j < 7; j++) {
						main.get()->push_back( std::make_shared<Fader>(Fader(channels[i]->at(j        ), std::make_shared<std::string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100)));
						ring1.get()->push_back(std::make_shared<Fader>(Fader(channels[i]->at(j + 7    ), std::make_shared<std::string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100)));
						ring2.get()->push_back(std::make_shared<Fader>(Fader(channels[i]->at(j + 7 * 2), std::make_shared<std::string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100)));
						ring3.get()->push_back(std::make_shared<Fader>(Fader(channels[i]->at(j + 7 * 3), std::make_shared<std::string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100)));
					}

					rings.get()->insert(rings.get()->begin(), std::make_shared<Column>(Column(std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {
						std::make_shared<Row>(Row(std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {
							std::make_shared<Row>(Row(main, std::make_shared<std::string>("main"), BorderSize::Single)),
								std::make_shared<Row>(Row(ring1, std::make_shared<std::string>("ring1"), BorderSize::Single))
						}), nullptr, BorderSize::None)),

						std::make_shared<Row>(Row(std::make_shared<std::vector<std::shared_ptr<Widget>>>(std::vector<std::shared_ptr<Widget>> {
							std::make_shared<Row>(Row(ring2, std::make_shared<std::string>("ring2"), BorderSize::Single)),
								std::make_shared<Row>(Row(ring3, std::make_shared<std::string>("ring3"), BorderSize::Single)),
						}), nullptr, BorderSize::None))
					}), std::make_shared<std::string>("light " + std::to_string(i + 1)), BorderSize::Double)));
				}
				state = State::Idle;
				break;
			}
		}
		((std::string*)input.get()->Text().get())->clear();
	}

	comm.Disconnect();

	return 0;
}



