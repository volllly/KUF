#pragma once
#include <windows.h>
#include <optional>
#include <functional>

#include "Protocol.h"

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
	std::shared_ptr<std::string> _title;
	BorderSize _border;

	virtual unsigned int InnerHeight();
	virtual unsigned int InnerWidth();

	virtual void DrawBorder(short int x, short int y);
	virtual void DrawContent(short int x, short int y) {}


public:
	Widget(std::shared_ptr<std::string> title, BorderSize border);

	unsigned int Height();
	unsigned int Width();

	void Draw(short int x, short int y);

	virtual void Focus() {}
};


class Interface
{
private:
	std::shared_ptr<Widget> _widget;
public:
	Interface(std::shared_ptr<Widget> widget);

	void Draw();
};

class Container : public Widget {
public:
	Container(std::shared_ptr<std::string> title, BorderSize border) : Widget(title, border) {}
};

class Row : public Container {
private:
	std::shared_ptr<std::vector<std::shared_ptr<Widget>>> _widgets;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Row(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets, std::shared_ptr<std::string> title, BorderSize border);
};

class Column : public Container {
private:
	std::shared_ptr<std::vector<std::shared_ptr<Widget>>> _widgets;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Column(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets, std::shared_ptr<std::string> title, BorderSize border);

};


class TextBox : public Widget {
private:
	std::shared_ptr<std::string> _text;

	unsigned int _width;
	unsigned int _height;
	int _x = 0;
	int _y = 0;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	TextBox(std::shared_ptr<std::string> text, std::shared_ptr<std::string> title, BorderSize border, unsigned int width, unsigned int height);
	std::shared_ptr<std::string> Text();

	void Focus();
};

class Fader : public Widget {
private:
	std::shared_ptr<double> _value;

	unsigned int _height;
	unsigned int _width;
	
	double _max;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);

public:
	Fader(std::shared_ptr<double> value, std::shared_ptr<std::string> title, BorderSize border, unsigned int width, unsigned int height, double max);
	std::shared_ptr<double> Value();
};