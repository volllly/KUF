#include "Protocol.h"

#include <numeric>
#include <functional>
#include <regex>

template <typename T> std::string join(std::vector<T> vector, std::string separator, std::function<std::string(T)> f, bool space = true) {
	std::string accumulator = "";

	for (auto& item : vector) {
		accumulator += f(item) + separator + (space ? " " : "");
	}
	
	if (space && accumulator.empty()) { accumulator.pop_back(); }

	return accumulator;
}

template <typename T, typename U> T parseEnum(U from, std::map<T, U> Attributes) {
	for (auto&& item : Attributes) {
		if (item.second == from) {
			return item.first;
		}
	}

	throw "Enum value not found";
}

std::string trim(const std::string& s)
{
	auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {return std::isspace(c); });
	auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return std::isspace(c); }).base();
	return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

std::vector<std::string> split(const std::string& s, std::string at) {
	std::vector<std::string> v = std::vector<std::string>{};

	std::string rest = s;
	for (;;) {
		int found = rest.find_first_of(at);
		std::string sub = rest.substr(0, found);
		if (sub.empty()) { break; }
		v.push_back(trim(sub));
		if (found == std::string::npos) { break; }
		rest = rest.substr(found + 1);
	}
	return v;
}

Value::Value(unsigned int light)
	: Value(light, std::nullopt) {}

Value::Value(unsigned int light, Ring ring)
	: Value(light, std::make_optional(ring)) {}

Value::Value(unsigned int light, Ring ring, Color color)
	: Value(light, std::make_optional(ring), std::make_optional(color)) {}

Value::Value(unsigned int light, Ring ring, Color color, double value)
	: Value(light, std::make_optional(ring), std::make_optional(color), std::make_optional(value))  {}


Value::Value(unsigned int light, std::optional<Ring> ring, std::optional<Color> color, std::optional<double> value) {
	_light = light;
	_ring = ring;
	_color = color;
	_value = value;
}

Value::Value(std::string from) {
	std::smatch valueMatch;


	if (!std::regex_match(from, valueMatch, std::regex("^(\\d*)(?: +(\\w*)(?: +(\\w*)(?: +(\\d*(?:\\.\\d*)?))?)?)?(?:.*)"))) {
		throw "could not parse values";
	}

	_light = std::stoul(valueMatch[1].str());
	_ring = valueMatch[2].matched ? std::optional(parseEnum<Ring, std::string>(valueMatch[2].str(), RingAttributes)) : std::nullopt;
	_color = valueMatch[3].matched ? std::optional(parseEnum<Color, std::string>(valueMatch[3].str(), ColorAttributes)) : std::nullopt;
	_value = valueMatch[4].matched ? std::optional(std::stod(valueMatch[4].str())) : std::nullopt;
}

std::string Value::ToString() {
	return std::to_string(_light) + (!_ring.has_value() ? "" : " " +
		RingAttributes.at(_ring.value()) + (!_color.has_value() ? "" : " " +
			ColorAttributes.at(_color.value()) + (!_value.has_value() ? "" : " " +
				std::to_string(_value.value()))));
}


unsigned int Value::GetLight() { return _light; }
std::optional<Ring> Value::GetRing() { return _ring; }
std::optional<Color> Value::GetColor() { return _color; }
std::optional<double> Value::GetValue() { return _value; }
void Value::SetValue(double value) {
	_value = std::make_optional(value);
};

std::string Message::Payload() {
	return "";
}

std::string Message::ToString() {
	std::string payload = Payload();
	return Head() + (payload.length() ? ": " + payload : "") + "\r\n";
}

Cmd Command::Cmd() {
	return _cmd;
}


std::string Command::Head() {
	return CmdAttributes.at(Cmd());
}


StatusCode Reply::StatusCode() {
	return _statusCode;
}

std::string Reply::Head() {
	return std::to_string(StatusCodeAttributes.at(StatusCode()));
}



Commands::Connect::Connect() {
	_cmd = Cmd::CONNECT;
	_versions = { "1" };
}

Commands::Connect::Connect(std::vector<std::string> versions) : Connect() {
	_versions = versions;

}



std::string Commands::Connect::Payload() {
	return std::accumulate(_versions.begin(), _versions.end(), std::string{}, [](std::string a, std::string b) {
		return a.length() && b.length() ? a + "; " + b : a + b;
	}) + ";";
}


Commands::Version::Version() {
	_cmd = Cmd::VERSION;
}


Commands::Set::Set(std::vector<Value> values) {
	_cmd = Cmd::SET;
	_values = values;
}

std::string Commands::Set::Payload() {
	return join<Value>(_values, ";", [](Value value) { return value.ToString(); });
}


Commands::Reset::Reset(std::vector<unsigned int> lights) {
	_cmd = Cmd::RESET;
	_lights = lights;
}

std::string Commands::Reset::Payload() {
	return join<unsigned int>(_lights, ";", [](unsigned int light) { return std::to_string(light); });

}


Commands::Status::Status(std::vector<unsigned int> status) {
	_cmd = Cmd::STATUS;
	_status = status;
}

std::string Commands::Status::Payload() {
	return join<unsigned int>(_status, ";", [](unsigned int status) { return std::to_string(status); });
}


Commands::Config::Config() {
	_cmd = Cmd::CONFIG;
}



Replies::Done::Done() {
	_statusCode = StatusCode::DONE;
}


Replies::Version::Version(std::string version) {
	_statusCode = StatusCode::VERSION;
	_version = version;
}

std::string Replies::Version::Payload() {
	return _version;
}


Replies::Config::Config(unsigned int lights) {
	_statusCode = StatusCode::CONFIG;
	_lights = lights;
}

std::string Replies::Config::Payload() {
	return std::to_string(_lights);
}

unsigned int Replies::Config::GetConfig() {
	return _lights;
}


Replies::StatusDifference::StatusDifference(std::vector<Value> status) {
	_statusCode = StatusCode::STATUS_DIFFERENCE;
	_status = status;
}

std::string Replies::StatusDifference::Payload() {
	return join<Value>(_status, ";", [](Value value) { return value.ToString(); });
}

std::vector<Value> Replies::StatusDifference::GetStatus() {
	return _status;
}

Replies::Status::Status(std::vector<Value> status) : StatusDifference(status) {
	_statusCode = StatusCode::STATUS;
}


Replies::NotFound::NotFound(std::vector<Value> status) {
	_statusCode = StatusCode::NOT_FOUND;
	_notFound = status;
}

std::string Replies::NotFound::Payload() {
	return join<Value>(_notFound, ";", [](Value value) { return value.ToString(); });
}


Replies::Unknown::Unknown(std::string command) {
	_statusCode = StatusCode::UNKNOWN;
	_command = command;
}

std::string Replies::Unknown::Payload() {
	return _command;
}


Replies::UnknownVersion::UnknownVersion(std::vector<std::string> versions) {
	_statusCode = StatusCode::UNKNOWN_VERSION;
	_versions = versions;
}

std::string Replies::UnknownVersion::Payload() {
	return join<std::string>(_versions, ";", [](std::string value) { return value; });
}


Replies::Internal::Internal(std::string details) {
	_statusCode = StatusCode::UNKNOWN;
	_details = details;
}

std::string Replies::Internal::Payload() {
	return _details;
}


Reply* MessageFactory::Reply(std::string from) {
	std::smatch statusCodeMatch;
	from= from.substr(0, from.find("\r\n"));

	if (!std::regex_match(from, statusCodeMatch, std::regex("([0-9]{3})(?: *: *(.*))?"))) { //^([0-9]{3})(?: *: *(.*))?
		throw "could not find StatusCode";
	}

	::StatusCode statusCode = parseEnum<::StatusCode, unsigned int>(std::stoul(statusCodeMatch[1].str()), StatusCodeAttributes);

	auto rest = trim(statusCodeMatch[2].str());

	switch (statusCode) {
	case StatusCode::DONE:
		return Replies::Done::Parse();
		break;
	case StatusCode::VERSION:
		return Replies::Version::Parse(rest);
		break;
	case StatusCode::CONFIG:
		return Replies::Config::Parse(rest);
		break;
	case StatusCode::STATUS_DIFFERENCE:
		return Replies::StatusDifference::Parse(rest);
		break;
	case StatusCode::STATUS:
		return Replies::Status::Parse(rest);
		break;
	case StatusCode::NOT_FOUND:
		return Replies::NotFound::Parse(rest);
		break;
	case StatusCode::UNKNOWN:
		return Replies::Unknown::Parse(rest);
		break;
	case StatusCode::UNKNOWN_VERSION:
		return Replies::UnknownVersion::Parse(rest);
		break;
	case StatusCode::INTERNAL:
		return Replies::Internal::Parse(rest);
		break;
	default:
		throw;
	}
}


Command* MessageFactory::Command(std::string from) {
	std::smatch cmdMatch;
	from = from.substr(0, from.find("\r\n"));

	if (!std::regex_match(from, cmdMatch, std::regex("^([a-z]*)(?: *: *(.*))?"))) {
		throw "could not find command";
	}

	::Cmd cmd = parseEnum<::Cmd, std::string>(cmdMatch[1].str(), CmdAttributes);

	auto rest = trim(cmdMatch[2].str());

	switch (cmd) {
	case Cmd::CONNECT:
		return Commands::Connect::Parse(rest);
		break;
	case Cmd::VERSION:
		return Commands::Version::Parse();
		break;
	case Cmd::CONFIG:
		return Commands::Config::Parse();
		break;
	case Cmd::SET:
		return Commands::Set::Parse(rest);
		break;
	case Cmd::RESET:
		return Commands::Reset::Parse(rest);
		break;
	case Cmd::STATUS:
		return Commands::Status::Parse(rest);
	default:
		throw;
	}
}

Replies::Done* Replies::Done::Parse() {
	return new Done();
}

Replies::Version* Replies::Version::Parse(std::string from) {
	return new Version(from);
}

Replies::Config* Replies::Config::Parse(std::string from) {
	return new Config(std::stoul(from));
}

Replies::StatusDifference* Replies::StatusDifference::Parse(std::string from) {
	std::vector<std::string> splitFrom = split(from, ";");
	std::vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return new Replies::StatusDifference(values);
}

Replies::Status* Replies::Status::Parse(std::string from) {
	std::vector<std::string> splitFrom = split(from, ";");
	std::vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return new Replies::Status(values);
}

Replies::NotFound* Replies::NotFound::Parse(std::string from) {
	std::vector<std::string> splitFrom = split(from, ";");
	std::vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return new Replies::NotFound(values);
}

Replies::Unknown* Replies::Unknown::Parse(std::string from) {
	return new Unknown(from);
}

Replies::UnknownVersion* Replies::UnknownVersion::Parse(std::string from) {
	return new Replies::UnknownVersion(split(from, ";"));
}

Replies::Internal* Replies::Internal::Parse(std::string from) {
	return new Internal(from);
}



Commands::Version* Commands::Version::Parse() {
	return new Commands::Version();
}

Commands::Connect* Commands::Connect::Parse(std::string from) {
	return new Commands::Connect(split(from, ";"));

}

Commands::Config* Commands::Config::Parse() {
	return new Commands::Config();
}

Commands::Set* Commands::Set::Parse(std::string from) {
	std::vector<std::string> splitFrom = split(from, ";");
	std::vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return new Commands::Set(values);
}

Commands::Reset* Commands::Reset::Parse(std::string from) {
	std::vector<std::string> splitFrom = split(from, ";");
	std::vector<unsigned int> values;
	for (auto& value : splitFrom) {
		values.push_back(std::stoul(value));
	}

	return new Commands::Reset(values);
}

Commands::Status* Commands::Status::Parse(std::string from) {
	std::vector<std::string> splitFrom = split(from, ";");
	std::vector<unsigned int> values;
	for (auto& value : splitFrom) {
		values.push_back(std::stoul(value));
	}

	return new Commands::Status(values);
}