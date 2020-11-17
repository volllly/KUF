#pragma once
#include <windows.h>
#include <optional>
#include <functional>

#include "Protocol.h"

class Binding {
protected:
	std::shared_ptr<void> _data;

	std::function<void(std::shared_ptr<void>)> _handler;

public:
	Binding(std::shared_ptr<void> data, std::function<void(std::shared_ptr<void>)> handler);

	std::shared_ptr<void> Get();

	void Update();
};

enum class BorderSize {
	None,
	Dotted,
	Dashed,
	Single,
	Double
};

const struct BorderSymbols {
	std::string tl;
	std::string tr;
	std::string h;
	std::string v;
	std::string bl;
	std::string br;
};

const std::map<BorderSize, BorderSymbols> BorderAttributes = {
	{BorderSize::Single, BorderSymbols {
		u8"┌",
		u8"┐",
		u8"─",
		u8"│",
		u8"└",
		u8"┘",
	}},
	{BorderSize::Dotted, BorderSymbols {
		u8"┌",
		u8"┐",
		u8"┈",
		u8"┊",
		u8"└",
		u8"┘",
	}},
	{BorderSize::Dashed, BorderSymbols {
		u8"┌",
		u8"┐",
		u8"┄",
		u8"┆",
		u8"└",
		u8"┘",
	}},
	{BorderSize::Double, BorderSymbols {
		u8"╔",
		u8"╗",
		u8"═",
		u8"║",
		u8"╚",
		u8"╝",
	}}
};

class Widget {
protected:
	std::optional<Binding> _binding;
	std::optional<std::string> _title;
	BorderSize _border;

	virtual unsigned int InnerHeight();
	virtual unsigned int InnerWidth();

	virtual void DrawBorder(short int x, short int y);
	virtual void DrawContent(short int x, short int y) {}


public:
	Widget(std::optional<Binding> binding, std::optional<std::string> title, BorderSize border);

	Binding* Binding();
	unsigned int Height();
	unsigned int Width();

	virtual void Input(INPUT_RECORD input) {}

	void Draw(short int x, short int y);

	virtual void Focus() {}

};


class Interface
{
private:
	std::shared_ptr<Widget> _widget;
public:
	Interface(std::shared_ptr<Widget> widget);

	void Input(INPUT_RECORD input);
	void Draw();
};

class Container : public Widget {
public:
	Container(std::optional<std::string> title, BorderSize border) : Widget(std::nullopt, title, border) {}
};

class Row : public Container {
private:
	std::vector<std::shared_ptr<Widget>> _widgets;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Row(std::vector<std::shared_ptr<Widget>> widgets, std::optional<std::string> title, BorderSize border);
};

class Column : public Container {
private:
	std::vector<std::shared_ptr<Widget>> _widgets;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Column(std::vector<std::shared_ptr<Widget>> widgets, std::optional<std::string> title, BorderSize border);

};


class TextBox : public Widget {
private:
	unsigned int _width;
	unsigned int _height;
	int _x = 0;
	int _y = 0;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	TextBox(::Binding binding, std::optional<std::string> title, BorderSize border, unsigned int width, unsigned int height);

	void Focus();
};

class Fader : public Widget {
private:
	unsigned int _height;
	unsigned int _width;
	
	double _max;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);

public:
	Fader(::Binding binding, std::optional<std::string> title, BorderSize border, unsigned int width, unsigned int height, double max);
};