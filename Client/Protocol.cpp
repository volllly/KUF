#include "Protocol.h"

#include <numeric>
#include <functional>
#include <regex>

template <typename T> string join(vector<T> vector, string separator, function<string(T)> f, bool space = true) {
	string accumulator = "";

	for (auto& item : vector) {
		accumulator += f(item) + separator + (space ? " " : "");
	}
	
	if (space && accumulator.empty()) { accumulator.pop_back(); }

	return accumulator;
}

template <typename T, typename U> T parseEnum(U from, map<T, U> Attributes) {
	for (auto&& item : Attributes) {
		if (item.second == from) {
			return item.first;
		}
	}

	throw "Enum value not found";
}

string trim(const string& s)
{
	auto wsfront = find_if_not(s.begin(), s.end(), [](int c) {return isspace(c); });
	auto wsback = find_if_not(s.rbegin(), s.rend(), [](int c) {return isspace(c); }).base();
	return (wsback <= wsfront ? string() : string(wsfront, wsback));
}

vector<string> split(const string& s, string at) {
	vector<string> v = vector<string>{};

	string rest = s;
	for (;;) {
		int found = rest.find_first_of(at);
		string sub = rest.substr(0, found);
		if (sub.empty()) { break; }
		v.push_back(trim(sub));
		if (found == string::npos) { break; }
		rest = rest.substr(found + 1);
	}
	return v;
}

Value::Value(unsigned int light)
	: Value(light, nullopt) {}

Value::Value(unsigned int light, Ring ring)
	: Value(light, make_optional(ring)) {}

Value::Value(unsigned int light, Ring ring, Color color)
	: Value(light, make_optional(ring), make_optional(color)) {}

Value::Value(unsigned int light, Ring ring, Color color, double value)
	: Value(light, make_optional(ring), make_optional(color), make_optional(value))  {}


Value::Value(unsigned int light, optional<Ring> ring, optional<Color> color, optional<double> value) {
	_light = light;
	_ring = ring;
	_color = color;
	_value = value;
}

Value::Value(string from) {
	smatch valueMatch;


	if (!regex_match(from, valueMatch, regex("^(\\d*)(?: +(\\w*)(?: +(\\w*)(?: +(\\d*(?:\\.\\d*)?))?)?)?(?:.*)"))) {
		throw "could not parse values";
	}

	_light = stoul(valueMatch[1].str());
	_ring = valueMatch[2].matched ? optional(parseEnum<Ring, string>(valueMatch[2].str(), RingAttributes)) : nullopt;
	_color = valueMatch[3].matched ? optional(parseEnum<Color, string>(valueMatch[3].str(), ColorAttributes)) : nullopt;
	_value = valueMatch[4].matched ? optional(stod(valueMatch[4].str())) : nullopt;
}

string Value::ToString() {
	return to_string(_light) + (!_ring.has_value() ? "" : " " +
		RingAttributes.at(_ring.value()) + (!_color.has_value() ? "" : " " +
			ColorAttributes.at(_color.value()) + (!_value.has_value() ? "" : " " +
				to_string(_value.value()))));
}


unsigned int Value::GetLight() { return _light; }
optional<Ring> Value::GetRing() { return _ring; }
optional<Color> Value::GetColor() { return _color; }
optional<double> Value::GetValue() { return _value; }
void Value::SetValue(double value) {
	_value = make_optional(value);
};

string Message::Payload() {
	return "";
}

string Message::ToString() {
	string payload = Payload();
	return Head() + (payload.length() ? ": " + payload : "") + "\r\n";
}

Cmd Command::Cmd() {
	return _cmd;
}


string Command::Head() {
	return CmdAttributes.at(Cmd());
}


StatusCode Reply::StatusCode() {
	return _statusCode;
}

string Reply::Head() {
	return to_string(StatusCodeAttributes.at(StatusCode()));
}



Commands::Connect::Connect() {
	_cmd = Cmd::CONNECT;
	_versions = { "1" };
}

Commands::Connect::Connect(vector<string> versions) : Connect() {
	_versions = versions;

}



string Commands::Connect::Payload() {
	return accumulate(_versions.begin(), _versions.end(), string{}, [](string a, string b) {
		return a.length() && b.length() ? a + "; " + b : a + b;
	}) + ";";
}


Commands::Version::Version() {
	_cmd = Cmd::VERSION;
}


Commands::Set::Set(vector<Value> values) {
	_cmd = Cmd::SET;
	_values = values;
}

string Commands::Set::Payload() {
	return join<Value>(_values, ";", [](Value value) { return value.ToString(); });
}


Commands::Reset::Reset(vector<unsigned int> lights) {
	_cmd = Cmd::RESET;
	_lights = lights;
}

string Commands::Reset::Payload() {
	return join<unsigned int>(_lights, ";", [](unsigned int light) { return to_string(light); });

}


Commands::Status::Status(vector<unsigned int> status) {
	_cmd = Cmd::STATUS;
	_status = status;
}

string Commands::Status::Payload() {
	return join<unsigned int>(_status, ";", [](unsigned int status) { return to_string(status); });
}


Commands::Config::Config() {
	_cmd = Cmd::CONFIG;
}



Replies::Done::Done() {
	_statusCode = StatusCode::DONE;
}


Replies::Version::Version(string version) {
	_statusCode = StatusCode::VERSION;
	_version = version;
}

string Replies::Version::Payload() {
	return _version;
}

string Replies::Version::GetVersion() {
	return _version;
}

Replies::Config::Config(unsigned int lights) {
	_statusCode = StatusCode::CONFIG;
	_lights = lights;
}

string Replies::Config::Payload() {
	return to_string(_lights);
}

unsigned int Replies::Config::GetConfig() {
	return _lights;
}


Replies::StatusDifference::StatusDifference(vector<Value> status) {
	_statusCode = StatusCode::STATUS_DIFFERENCE;
	_status = status;
}

string Replies::StatusDifference::Payload() {
	return join<Value>(_status, ";", [](Value value) { return value.ToString(); });
}

vector<Value> Replies::StatusDifference::GetStatus() {
	return _status;
}

Replies::Status::Status(vector<Value> status) : StatusDifference(status) {
	_statusCode = StatusCode::STATUS;
}


Replies::NotFound::NotFound(vector<Value> status) {
	_statusCode = StatusCode::NOT_FOUND;
	_notFound = status;
}

string Replies::NotFound::Payload() {
	return join<Value>(_notFound, ";", [](Value value) { return value.ToString(); });
}


Replies::Unknown::Unknown(string command) {
	_statusCode = StatusCode::UNKNOWN;
	_command = command;
}

string Replies::Unknown::Payload() {
	return _command;
}


Replies::UnknownVersion::UnknownVersion(vector<string> versions) {
	_statusCode = StatusCode::UNKNOWN_VERSION;
	_versions = versions;
}

string Replies::UnknownVersion::Payload() {
	return join<string>(_versions, ";", [](string value) { return value; });
}


Replies::Internal::Internal(string details) {
	_statusCode = StatusCode::UNKNOWN;
	_details = details;
}

string Replies::Internal::Payload() {
	return _details;
}


shared_ptr<Reply> MessageFactory::Reply(string from) {
	smatch statusCodeMatch;
	from= from.substr(0, from.find("\r\n"));

	if (!regex_match(from, statusCodeMatch, regex("([0-9]{3})(?: *: *(.*))?"))) { //^([0-9]{3})(?: *: *(.*))?
		throw "could not find StatusCode";
	}

	::StatusCode statusCode = parseEnum<::StatusCode, unsigned int>(stoul(statusCodeMatch[1].str()), StatusCodeAttributes);

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


shared_ptr<Command> MessageFactory::Command(string from) {
	smatch cmdMatch;
	from = from.substr(0, from.find("\r\n"));

	if (!regex_match(from, cmdMatch, regex("^([a-z]*)(?: *: *(.*))?"))) {
		throw "could not find command";
	}

	::Cmd cmd = parseEnum<::Cmd, string>(cmdMatch[1].str(), CmdAttributes);

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

shared_ptr<Replies::Done> Replies::Done::Parse() {
	return make_shared<Done>();
}

shared_ptr<Replies::Version> Replies::Version::Parse(string from) {
	return make_shared<Version>(from);
}

shared_ptr<Replies::Config> Replies::Config::Parse(string from) {
	return make_shared<Config>(stoul(from));
}

shared_ptr<Replies::StatusDifference> Replies::StatusDifference::Parse(string from) {
	vector<string> splitFrom = split(from, ";");
	vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return make_shared<StatusDifference>(values);
}

shared_ptr<Replies::Status> Replies::Status::Parse(string from) {
	vector<string> splitFrom = split(from, ";");
	vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return make_shared<Status>(values);
}

shared_ptr<Replies::NotFound> Replies::NotFound::Parse(string from) {
	vector<string> splitFrom = split(from, ";");
	vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return make_shared<NotFound>(values);
}

shared_ptr<Replies::Unknown> Replies::Unknown::Parse(string from) {
	return make_shared<Unknown>(from);
}

shared_ptr<Replies::UnknownVersion> Replies::UnknownVersion::Parse(string from) {
	return make_shared<UnknownVersion>(split(from, ";"));
}

shared_ptr<Replies::Internal> Replies::Internal::Parse(string from) {
	return make_shared<Internal>(from);
}



shared_ptr<Commands::Version> Commands::Version::Parse() {
	return make_shared<Version>();
}

shared_ptr<Commands::Connect> Commands::Connect::Parse(string from) {
	return make_shared<Connect>(split(from, ";"));

}

shared_ptr<Commands::Config> Commands::Config::Parse() {
	return make_shared<Config>();
}

shared_ptr<Commands::Set> Commands::Set::Parse(string from) {
	vector<string> splitFrom = split(from, ";");
	vector<Value> values;
	for (auto& value : splitFrom) {
		values.push_back(Value(value));
	}

	return make_shared<Set>(values);
}

shared_ptr<Commands::Reset> Commands::Reset::Parse(string from) {
	vector<string> splitFrom = split(from, ";");
	vector<unsigned int> values;
	for (auto& value : splitFrom) {
		values.push_back(stoul(value));
	}

	return make_shared<Reset>(values);
}

shared_ptr<Commands::Status> Commands::Status::Parse(string from) {
	vector<string> splitFrom = split(from, ";");
	vector<unsigned int> values;
	for (auto& value : splitFrom) {
		values.push_back(stoul(value));
	}

	return make_shared<Status>(values);
}