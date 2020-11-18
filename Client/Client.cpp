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
	shared_ptr<string> receiver = make_shared<string>("");
	shared_ptr<CommCallbacks> callback = make_shared<CallbackHandler>(receiver);
	Communication comm(callback);

	bool res = comm.Connect(servername, serverPort);

	vector<shared_ptr<vector<shared_ptr<double>>>> channels = vector<shared_ptr<vector<shared_ptr<double>>>>{};

	shared_ptr<TextBox> status = make_shared<TextBox>(make_shared<string>(""), make_shared<string>("log"), BorderSize::Double, 88, 6);


	shared_ptr<TextBox> input = make_shared<TextBox>(make_shared<string>(""), nullptr, BorderSize::Single, 88, 1);

	shared_ptr<vector<shared_ptr<Widget>>> rings = make_shared<vector<shared_ptr<Widget>>>(vector<shared_ptr<Widget>> {
		status,
		input
	});

	Interface tui = Interface(std::make_shared<Column>(rings, nullptr, BorderSize::None));
	
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

		shared_ptr<Reply> reply(nullptr);

		string send;
		switch (state) {
		case State::Idle:
		{
			input->Focus();

			string read = *input->Text();

			status->Text()->append("\n> " + read);

			if (read.find("quit") == 0) {
				comm.Disconnect();
				return 0;
			}

			send = messageFactory.Command(read)->ToString();
			break;
		}
		case State::Configure:
			send = Commands::Config().ToString();

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
				vector<Value> values = ((Replies::StatusDifference*)reply.get())->GetStatus();

				for (auto& value : values) {
					*channels[value.GetLight() - 1]->at(as_integer(value.GetRing().value()) * 7 + as_integer(value.GetColor().value())) = value.GetValue().value();
				}
				break;
			}
			case StatusCode::CONFIG:
				for (int i = 0; i < ((Replies::Config*)reply.get())->GetConfig(); i++) {
					shared_ptr<vector<shared_ptr<double>>> channel = make_shared<vector<shared_ptr<double>>>();
					for (int j = 0; j < 7 * 4; j++) {
						channel->push_back(make_shared<double>(0));
					}
					channels.push_back(channel);
				}

				const string names[] = { "D", "R", "G", "B", "W", "A", "U" };

				for (int i = ((Replies::Config*)reply.get())->GetConfig() - 1; i >= 0; i--) {
					auto light = make_shared<Column>(Column(make_shared<vector<shared_ptr<Widget>>>(), make_shared<string>("ring3"), BorderSize::Double));

					shared_ptr<vector<shared_ptr<Widget>>> main = make_shared<vector<shared_ptr<Widget>>>();
					shared_ptr<vector<shared_ptr<Widget>>> ring1 = make_shared<vector<shared_ptr<Widget>>>();
					shared_ptr<vector<shared_ptr<Widget>>> ring2 = make_shared<vector<shared_ptr<Widget>>>();
					shared_ptr<vector<shared_ptr<Widget>>> ring3 = make_shared<vector<shared_ptr<Widget>>>();

					for (int j = 0; j < 7; j++) {
						main->push_back(make_shared<Fader>(channels[i]->at(j), make_shared<string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100));
						ring1->push_back(make_shared<Fader>(channels[i]->at(j + 7), make_shared<string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100));
						ring2->push_back(make_shared<Fader>(channels[i]->at(j + 7 * 2), make_shared<string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100));
						ring3->push_back(make_shared<Fader>(channels[i]->at(j + 7 * 3), make_shared<string>(names[j % 7]), j % 7 ? BorderSize::Dashed : BorderSize::None, 4, 6, 100));
					}

					rings->insert(rings->begin(), make_shared<Column>(make_shared<vector<shared_ptr<Widget>>>(initializer_list<shared_ptr<Widget>> {
						make_shared<Row>(make_shared<vector<shared_ptr<Widget>>>(initializer_list<shared_ptr<Widget>> {
							make_shared<Row>(main, make_shared<string>("main"), BorderSize::Single),
								make_shared<Row>(ring1, make_shared<string>("ring1"), BorderSize::Single)
						}), nullptr, BorderSize::None),

							make_shared<Row>(make_shared<vector<shared_ptr<Widget>>>(initializer_list<shared_ptr<Widget>> {
								make_shared<Row>(ring2, make_shared<string>("ring2"), BorderSize::Single),
									make_shared<Row>(ring3, make_shared<string>("ring3"), BorderSize::Single),
							}), nullptr, BorderSize::None)
					}), make_shared<string>("light " + to_string(i + 1)), BorderSize::Double));
				}
				state = State::Idle;
				break;
			}
		}
		input->Text()->clear();
	}
}



