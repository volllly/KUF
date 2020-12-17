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
-> typename underlying_type<Enumeration>::type
{
	return static_cast<typename underlying_type<Enumeration>::type>(value);
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
	auto receiver = make_shared<string>("");
	auto callback = make_shared<CallbackHandler>(receiver);
	Communication comm(callback);

	bool res = comm.Connect(servername, serverPort);

	vector<unique_ptr<vector<shared_ptr<double>>>> channels {};

	auto status = make_shared<TextBox>("", "log", BorderSize::Double, 84, 6);


	auto input = make_shared<TextBox>("", shared_ptr<string>({}), BorderSize::Single, 84, 1);

	auto rings = make_shared<vector<shared_ptr<Widget>>>(initializer_list<shared_ptr<Widget>> {
		status,
		input
	});

	Interface tui(Column(rings, shared_ptr<string>({}), BorderSize::None));
	
	if (!res) {
		cerr << "error connecting;" << endl;
	}

	enum class State {
		Connect,
		Configure,
		Idle
	};
	auto state = State::Connect;

	MessageFactory messageFactory{};


	for (;;) {
		tui.Draw();

		shared_ptr<Reply> reply(nullptr);

		string send;
		shared_ptr<Command> command;
		switch (state) {
		case State::Idle:
		{
			input->Focus();

			auto read = *input->Text();

			status->Text()->append("\n> " + read);

			if (read.find("quit") == 0) {
				comm.Disconnect();
				return 0;
			}

			command = messageFactory.Command(read);
			send = command->ToString();
			break;
		}
		case State::Configure:
			send = Commands::Config(map<unsigned int, unsigned int>{ {1, 1} }).ToString();

			status->Text()->append("\n> " + send.substr(0, send.length() - 2));
			break;
		case State::Connect:
			send = Commands::Connect().ToString();

			status->Text()->append("\n> " + send.substr(0, send.length() - 2));
			break;
		}
		comm.WriteToPartner(send.c_str(), send.length());

		while (!comm.IsMessagePending()) {
			__noop();
		}

		while (comm.IsMessagePending()) {
			comm.ProcessMessage();
			if (receiver->empty()) {
				while (!comm.IsMessagePending()) {
					__noop();
				}
				continue;
			}
			reply = messageFactory.Reply(*receiver);
			status->Text()->append("\n< " + reply->ToString().substr(0, reply->ToString().length() - 2));
			switch (reply->StatusCode()) {
			case StatusCode::DONE:
				if (command->Cmd() == Cmd::SET) {
					auto values = ((Commands::Set*)command.get())->GetValues();
					for (auto& value : values) {
						*channels[value.GetLight() - 1]->at(as_integer(value.GetRing().value()) * 6 + as_integer(value.GetColor().value())) = value.GetValue().value();
					}
				}
				break;
			case StatusCode::VERSION:
				if (((Replies::Version*)reply.get())->GetVersion() != "1") {
					return 1;
				}
				if (state == State::Connect) {
					state = State::Configure;
				}
				break;
			case StatusCode::STATUS:
			case StatusCode::STATUS_DIFFERENCE:
			{
				auto values = ((Replies::StatusDifference*)reply.get())->GetStatus();

				for (auto& value : values) {
					if (as_integer(value.GetColor().value()) < 6) {
						*channels[value.GetLight() - 1]->at(as_integer(value.GetRing().value()) * 6 + as_integer(value.GetColor().value())) = value.GetValue().value();
					}
					else {
						for (int i = 0; i < 6; i++) {
							*channels[value.GetLight() - 1]->at(as_integer(value.GetRing().value()) * 6 + i) = -1;
						}
					}
				}
				break;
			}
			case StatusCode::CONFIG:
				for (int i = 0; i < (signed)((Replies::Config*)reply.get())->GetConfig().size(); i++) {
					auto channel = make_unique<vector<shared_ptr<double>>>();
					for (int j = 0; j < 6 * 4; j++) {
						channel->push_back(make_shared<double>(0));
					}
					channels.push_back(move(channel));
				}

				const string names[] = { "R", "G", "B", "W", "A", "U" };

				for (int i = ((Replies::Config*)reply.get())->GetConfig().size() - 1; i >= 0; i--) {
					auto light = make_shared<Column>(vector<shared_ptr<Widget>>{}, "ring3", BorderSize::Double);

					auto main = make_shared<vector<shared_ptr<Widget>>>();
					auto ring1 = make_shared<vector<shared_ptr<Widget>>>();
					auto ring2 = make_shared<vector<shared_ptr<Widget>>>();
					auto ring3 = make_shared<vector<shared_ptr<Widget>>>();

					for (int j = 0; j < 6; j++) {
						main->push_back(make_shared<Fader>(channels[i]->at(j), string(names[j % 6]), BorderSize::Dashed, 4, 6, 100));
						ring1->push_back(make_shared<Fader>(channels[i]->at(j + 6), string(names[j % 6]), BorderSize::Dashed, 4, 6, 100));
						ring2->push_back(make_shared<Fader>(channels[i]->at(j + 6 * 2), string(names[j % 6]), BorderSize::Dashed, 4, 6, 100));
						ring3->push_back(make_shared<Fader>(channels[i]->at(j + 6 * 3), string(names[j % 6]), BorderSize::Dashed, 4, 6, 100));
					}

					rings->insert(rings->begin(), make_shared<Column>(initializer_list<shared_ptr<Widget>> {
						make_shared<Row>(initializer_list<shared_ptr<Widget>> {
							make_shared<Row>(main, "main", BorderSize::Single),
							make_shared<Row>(ring1, "ring1", BorderSize::Single)
						}, shared_ptr<string>({}), BorderSize::None),

						make_shared<Row>(initializer_list<shared_ptr<Widget>> {
							make_shared<Row>(ring2, "ring2", BorderSize::Single),
							make_shared<Row>(ring3, "ring3", BorderSize::Single),
						}, shared_ptr<string>({}), BorderSize::None)
					}, "light " + to_string(i + 1), BorderSize::Double));
				}
				state = State::Idle;
				break;
			}
		}
		input->Text()->clear();
	}
}



