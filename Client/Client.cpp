// Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CallbackHandler.h"
#include "Protocol.h"
#include "Interface.h"

#include <conio.h>

#include <iostream>

#undef SHOWMESSAGE

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

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

	auto status = make_shared<TextBox>("", "log", BorderSize::Double, 88, 6, true);


	auto input = make_shared<TextBox>("", shared_ptr<string>({}), BorderSize::Single, 88, 1, false);

	auto rings = make_shared<vector<shared_ptr<Widget>>>(initializer_list<shared_ptr<Widget>> {
		status,
		input
	});

	auto maincol = make_shared<Column>(rings, shared_ptr<string>({}), BorderSize::None);
	shared_ptr<Interface> tui = make_shared<Interface>((shared_ptr<Widget>)maincol);
	
	if (!res) {
		cerr << "error connecting;" << endl;
	}

	MessageFactory messageFactory{};

	function<void(shared_ptr<Command> command)> processor = [&](shared_ptr<Command> command){
		shared_ptr<Reply> reply(nullptr);

		string send;

		send = command->ToString();

		status->Text()->append("\n> " + send.substr(0, send.length() - 2));

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

			try {
				reply = messageFactory.Reply(*receiver);
			}
			catch (const exception& e) {
				status->Text()->append("\n| " + *receiver);
				status->Text()->append("\nx ");
				status->Text()->append(e.what());
				continue;
			}
			catch (...) {
				status->Text()->append("\n| " + *receiver);
				status->Text()->append("\nx Error reading reply");
				continue;
			}

			status->Text()->append("\n< " + reply->ToString().substr(0, reply->ToString().length() - 2));
			switch (reply->StatusCode()) {
			case StatusCode::DONE:
				if (command->Cmd() == Cmd::SET) {
					auto values = ((Commands::Set*)command.get())->GetValues();
					for (auto& value : values) {
						if (value.GetRing().value() == Ring::RING) {
							if (value.GetColor().value() == Color::STROBE) {
								*channels[value.GetLight() - 1]->at(1 * 7) = value.GetValue().value();
								*channels[value.GetLight() - 1]->at(2 * 7) = value.GetValue().value();
								*channels[value.GetLight() - 1]->at(3 * 7) = value.GetValue().value();
							}
						}
						else {
							*channels[value.GetLight() - 1]->at(as_integer(value.GetRing().value()) * 7 + as_integer(value.GetColor().value())) = value.GetValue().value();
						}
					}
				}
				break;
			case StatusCode::VERSION:
				if (((Replies::Version*)reply.get())->GetVersion() != "1") {
					status->Text()->append("\nx Unknown version");
					return;
				}
				break;
			case StatusCode::STATUS:
			case StatusCode::STATUS_DIFFERENCE:
			{
				auto values = ((Replies::StatusDifference*)reply.get())->GetStatus();

				for (auto& value : values) {
					if (value.GetRing().value() == Ring::RING) {
						if (value.GetColor().value() == Color::STROBE) {
							*channels[value.GetLight() - 1]->at(1 * 7) = value.GetValue().value();
							*channels[value.GetLight() - 1]->at(2 * 7) = value.GetValue().value();
							*channels[value.GetLight() - 1]->at(3 * 7) = value.GetValue().value();
						}
					}
					else {
						*channels[value.GetLight() - 1]->at(as_integer(value.GetRing().value()) * 7 + as_integer(value.GetColor().value())) = value.GetValue().value();
					}
				}
				break;
			}
			case StatusCode::CONFIG:
				for (int i = 0; i < (signed)((Replies::Config*)reply.get())->GetConfig().size(); i++) {
					auto channel = make_unique<vector<shared_ptr<double>>>();
					for (int j = 0; j < 7 * 4; j++) {
						channel->push_back(make_shared<double>(0));
					}
					channels.push_back(move(channel));
				}

				const string names[] = { "S", "R", "G", "B", "W", "A", "U" };
				const Color colors[7] = { Color::STROBE, Color::RED, Color::GREEN, Color::BLUE, Color::WHITE, Color::AMBER, Color::UV, };

				for (int i = ((Replies::Config*)reply.get())->GetConfig().size() - 1; i >= 0; i--) {
					auto light = make_shared<Column>(vector<shared_ptr<Widget>>{}, "ring3", BorderSize::Double);

					auto main = make_shared<vector<shared_ptr<Widget>>>();
					auto ring1 = make_shared<vector<shared_ptr<Widget>>>();
					auto ring2 = make_shared<vector<shared_ptr<Widget>>>();
					auto ring3 = make_shared<vector<shared_ptr<Widget>>>();

					for (int j = 0; j < 7; j++) {
						auto fm = make_shared<Fader>(channels[i]->at(j), string(names[j % 7]), j % 7 == 0 ? BorderSize::None : BorderSize::Dashed, 4, 7, 100);
						fm->OnChange([=, &processor](shared_ptr<Fader> f) {
							processor(make_shared<Commands::Set>(initializer_list{ Value(i + 1, Ring::MAIN, colors[j % 7], *f->Value()) }));
						});
						main->push_back(fm);

						auto r1 = make_shared<Fader>(channels[i]->at(j + 7), string(names[j % 7]), j % 7 == 0 ? BorderSize::None : BorderSize::Dashed, 4, 7, 100);
						r1->OnChange([=, &processor](shared_ptr<Fader> f) {
							processor(make_shared<Commands::Set>(initializer_list{ Value(i + 1, Ring::RING1, colors[j % 7], *f->Value()) }));
						});
						ring1->push_back(r1);

						auto r2 = make_shared<Fader>(channels[i]->at(j + 7 * 2), string(names[j % 7]), j % 7 == 0 ? BorderSize::None : BorderSize::Dashed, 4, 7, 100);
						r2->OnChange([=, &processor](shared_ptr<Fader> f) {
							processor(make_shared<Commands::Set>(initializer_list{ Value(i + 1, Ring::RING2, colors[j % 7], *f->Value()) }));
						});
						ring2->push_back(r2);

						auto r3 = make_shared<Fader>(channels[i]->at(j + 7 * 3), string(names[j % 7]), j % 7 == 0 ? BorderSize::None : BorderSize::Dashed, 4, 7, 100);
						r3->OnChange([=, &processor](shared_ptr<Fader> f) {
							processor(make_shared<Commands::Set>(initializer_list{ Value(i + 1, Ring::RING3, colors[j % 7], *f->Value()) }));
						});
						ring3->push_back(r3);
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

				maincol->Updated();
				break;
			}
		}
	};

	input->OnChange([&](shared_ptr<TextBox> w) {
		shared_ptr<Command> command;
		string read = *w->Text();

		try {
			command = messageFactory.Command(read);
		}
		catch (const exception& e) {
			status->Text()->append("\n| " + read);
			status->Text()->append("\nx ");
			status->Text()->append(e.what());
			*w->Text() = "";
			return;
		}
		catch (...) {
			status->Text()->append("\n| " + read);
			status->Text()->append("\nx Error reading command");
			*w->Text() = "";
			return;
		}

		processor(command);
		*w->Text() = "";
	});

	processor(make_shared<Commands::Connect>());

	tui->Run();
	exit(0);
}



