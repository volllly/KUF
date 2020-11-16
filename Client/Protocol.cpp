#include "Protocol.h"

#include <numeric>
#include <functional>

template <typename T> std::string join(std::vector<T> vector, std::string separator, std::function<std::string(T)> f, bool space = true) {
	std::string accumulator = "";

	for (auto& item : vector) {
		accumulator += f(item) + separator + (space ? " " : "");
	}
	
	if (space && accumulator.empty()) { accumulator.pop_back(); }

	return accumulator;
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

std::string Value::ToString() {
	return std::to_string(_light) + (!_ring.has_value() ? "" : " " +
		RingAttributes.at(_ring.value()) + (!_color.has_value() ? "" : " " +
			ColorAttributes.at(_color.value()) + (!_value.has_value() ? "" : " " +
				std::to_string(_value.value()))));
}


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


Replies::StatusDifference::StatusDifference(std::vector<Value> status) {
	_statusCode = StatusCode::STATUS_DIFFERENCE;
	_status = status;
}

std::string Replies::StatusDifference::Payload() {
	return join<Value>(_status, ";", [](Value value) { return value.ToString(); });
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

