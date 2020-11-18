#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <iostream>

using namespace std;

enum class Ring {
	MAIN,
	RING1,
	RING2,
	RING3
};
const map<Ring, string> RingAttributes = {
	{Ring::MAIN,  "main"},
	{Ring::RING1, "ring1"},
	{Ring::RING2, "ring2"},
	{Ring::RING3, "ring3"},
};

enum class Color {
	DIMM,
	RED,
	GREEN,
	BLUE,
	WHITE,
	AMBER,
	UV
};
const map<Color, string> ColorAttributes = {
	{Color::DIMM,  "dimm"},
	{Color::RED,   "red"},
	{Color::GREEN, "green"},
	{Color::BLUE,  "blue"},
	{Color::WHITE, "white"},
	{Color::AMBER, "amber"},
	{Color::UV,    "uv"},
};


enum class StatusCode {
	DONE,
	VERSION,
	CONFIG,
	STATUS_DIFFERENCE,
	STATUS,
	NOT_FOUND,
	UNKNOWN,
	UNKNOWN_VERSION,
	INTERNAL
};
const map<StatusCode, unsigned int> StatusCodeAttributes = {
	{StatusCode::DONE,				200},
	{StatusCode::VERSION,			210},
	{StatusCode::CONFIG,			211},
	{StatusCode::STATUS_DIFFERENCE, 202},
	{StatusCode::STATUS,			203},
	{StatusCode::NOT_FOUND,			300},
	{StatusCode::UNKNOWN,			400},
	{StatusCode::UNKNOWN_VERSION,	401},
	{StatusCode::INTERNAL,			500},
};

enum class Cmd {
	CONNECT,
	VERSION,
	SET,
	RESET,
	STATUS,
	CONFIG
};
const map<Cmd, string> CmdAttributes = {
	{Cmd::CONNECT, "connect"},
	{Cmd::VERSION, "version"},
	{Cmd::SET,     "set"},
	{Cmd::RESET,   "reset"},
	{Cmd::STATUS,  "status"},
	{Cmd::CONFIG,  "config"},
};

class Value
{
private:
	unsigned int _light;
	optional<Ring> _ring;
	optional<Color> _color;
	optional<double> _value;

	Value(unsigned int light, optional<Ring> ring, optional<Color> color = nullopt, optional<double> value = nullopt);

public:
	Value(unsigned int light);
	Value(unsigned int light, Ring ring);
	Value(unsigned int light, Ring ring, Color color);
	Value(unsigned int light, Ring ring, Color color, double value);

	Value(string);

	string ToString();

	unsigned int GetLight();
	optional<Ring> GetRing();
	optional<Color> GetColor();
	optional<double> GetValue();
	void SetValue(double);
};

class Message {
protected:
	virtual string Head() = 0;

public:
	virtual string ToString();
	virtual string Payload();
};

class Command : public Message {
protected:
	Cmd _cmd;

	string Head();

public:
	Cmd Cmd();
};

class Reply : public Message {
protected:
	StatusCode _statusCode;

	string Head();

public:
	StatusCode StatusCode();
};

namespace Commands {
	class Connect : public Command
	{
	private:
		vector<string> _versions;
		string Payload();
		
	public:
		Connect();
		Connect(vector<string> versions);
		static shared_ptr<Connect> Parse(string from);
	};

	class Version : public Command
	{
	public:
		Version();
		static shared_ptr<Version> Parse();
	};

	class Set : public Command
	{
	private:
		vector<Value> _values;
		string Payload();
	public:
		Set(vector<Value> values);
		static shared_ptr<Set> Parse(string from);
	};

	class Reset : public Command
	{
	private:
		vector<unsigned int> _lights;
		string Payload();
	public:
		Reset(vector<unsigned int> lights);
		static shared_ptr<Reset> Parse(string from);
	};

	class Status : public Command
	{
	private:
		vector<unsigned int> _status;
		string Payload();
	public:
		Status(vector<unsigned int> lights);
		static shared_ptr<Status> Parse(string from);
	};

	class Config : public Command
	{
	public:
		Config();
		static shared_ptr<Config> Parse();
	};
}

namespace Replies {
	class Done : public Reply
	{
	public:
		Done();
		static shared_ptr<Done> Parse();
	};

	class Version : public Reply
	{
	private:
		string _version;
		string Payload();
	public:
		Version(string version);
		static shared_ptr<Version> Parse(string from);
		string GetVersion();
	};

	class Config : public Reply
	{
	private:
		unsigned int _lights;
		string Payload();
	public:
		Config(unsigned int lights);
		static shared_ptr<Config> Parse(string from);
		unsigned int GetConfig();

	};

	class StatusDifference : public Reply
	{
	private:
		vector<Value> _status;
		string Payload();
	public:
		StatusDifference(vector<Value> status);
		static shared_ptr<StatusDifference> Parse(string from);
		vector<Value> GetStatus();
	};

	class Status : public StatusDifference {
	public:
		Status(vector<Value> status);
		static shared_ptr<Status> Parse(string from);
	};

	class NotFound : public Reply {
	private:
		vector<Value> _notFound;
		string Payload();
	public:
		NotFound(vector<Value> notFound);
		static shared_ptr<NotFound> Parse(string from);
	};

	class Unknown : public Reply
	{
	private:
		string _command;
		string Payload();
	public:
		Unknown(string command);
		static shared_ptr<Unknown> Parse(string from);
	};

	class UnknownVersion : public Reply
	{
	private:
		vector<string> _versions;
		string Payload();
	public:
		UnknownVersion(vector<string> versions);
		static shared_ptr<UnknownVersion> Parse(string from);
	};

	class Internal : public Reply
	{
	private:
		string _details;
		string Payload();
	public:
		Internal(string details);
		static shared_ptr<Internal> Parse(string from);
	};
}


class MessageFactory {
public:
	shared_ptr<Command> Command(string from);

	shared_ptr<Reply> Reply(string from);
};