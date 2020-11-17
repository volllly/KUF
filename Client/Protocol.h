#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <iostream>

enum class Ring {
	MAIN,
	RING1,
	RING2,
	RING3
};
const std::map<Ring, std::string> RingAttributes = {
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
const std::map<Color, std::string> ColorAttributes = {
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
const std::map<StatusCode, unsigned int> StatusCodeAttributes = {
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
const std::map<Cmd, std::string> CmdAttributes = {
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
	std::optional<Ring> _ring;
	std::optional<Color> _color;
	std::optional<double> _value;

	Value(unsigned int light, std::optional<Ring> ring, std::optional<Color> color = std::nullopt, std::optional<double> value = std::nullopt);

public:
	Value(unsigned int light);
	Value(unsigned int light, Ring ring);
	Value(unsigned int light, Ring ring, Color color);
	Value(unsigned int light, Ring ring, Color color, double value);

	Value(std::string);

	std::string ToString();

	unsigned int GetLight();
	std::optional<Ring> GetRing();
	std::optional<Color> GetColor();
	std::optional<double> GetValue();
	void SetValue(double);
};

class Message {
protected:
	virtual std::string Head() = 0;

public:
	virtual std::string ToString();
	virtual std::string Payload();

	~Message() {
		std::cout << "deconstructed";
	}
};

class Command : public Message {
protected:
	Cmd _cmd;

	std::string Head();

public:
	Cmd Cmd();
};

class Reply : public Message {
protected:
	StatusCode _statusCode;

	std::string Head();

public:
	StatusCode StatusCode();
};

namespace Commands {
	class Connect : public Command
	{
	private:
		std::vector<std::string> _versions;
		std::string Payload();
		
	public:
		Connect();
		Connect(std::vector<std::string> versions);
		static Connect* Parse(std::string from);
	};

	class Version : public Command
	{
	public:
		Version();
		static Version* Parse();
	};

	class Set : public Command
	{
	private:
		std::vector<Value> _values;
		std::string Payload();
	public:
		Set(std::vector<Value> values);
		static Set* Parse(std::string from);
	};

	class Reset : public Command
	{
	private:
		std::vector<unsigned int> _lights;
		std::string Payload();
	public:
		Reset(std::vector<unsigned int> lights);
		static Reset* Parse(std::string from);
	};

	class Status : public Command
	{
	private:
		std::vector<unsigned int> _status;
		std::string Payload();
	public:
		Status(std::vector<unsigned int> lights);
		static Status* Parse(std::string from);
	};

	class Config : public Command
	{
	public:
		Config();
		static Config* Parse();
	};
}

namespace Replies {
	class Done : public Reply
	{
	public:
		Done();
		static Done* Parse();
	};

	class Version : public Reply
	{
	private:
		std::string _version;
		std::string Payload();
	public:
		Version(std::string version);
		static Version* Parse(std::string from);
	};

	class Config : public Reply
	{
	private:
		unsigned int _lights;
		std::string Payload();
	public:
		Config(unsigned int lights);
		static Config* Parse(std::string from);
		unsigned int GetConfig();

	};

	class StatusDifference : public Reply
	{
	private:
		std::vector<Value> _status;
		std::string Payload();
	public:
		StatusDifference(std::vector<Value> status);
		static StatusDifference* Parse(std::string from);
		std::vector<Value> GetStatus();
	};

	class Status : public StatusDifference {
	public:
		Status(std::vector<Value> status);
		static Status* Parse(std::string from);
	};

	class NotFound : public Reply {
	private:
		std::vector<Value> _notFound;
		std::string Payload();
	public:
		NotFound(std::vector<Value> notFound);
		static NotFound* Parse(std::string from);
	};

	class Unknown : public Reply
	{
	private:
		std::string _command;
		std::string Payload();
	public:
		Unknown(std::string command);
		static Unknown* Parse(std::string from);
	};

	class UnknownVersion : public Reply
	{
	private:
		std::vector<std::string> _versions;
		std::string Payload();
	public:
		UnknownVersion(std::vector<std::string> versions);
		static UnknownVersion* Parse(std::string from);
	};

	class Internal : public Reply
	{
	private:
		std::string _details;
		std::string Payload();
	public:
		Internal(std::string details);
		static Internal* Parse(std::string from);
	};
}


class MessageFactory {
public:
	Command* Command(std::string from);

	Reply* Reply(std::string from);
};