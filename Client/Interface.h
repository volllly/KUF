#pragma once
#include <windows.h>
#include <optional>
#include <functional>

#include "Protocol.h"

using namespace std;

enum class BorderSize {
	None,
	Dotted,
	Dashed,
	Single,
	Double
};

const struct BorderSymbols {
	string tl;
	string tr;
	string h;
	string v;
	string bl;
	string br;
};

const map<BorderSize, BorderSymbols> BorderAttributes = {
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
	shared_ptr<string> _title;
	BorderSize _border;

	virtual unsigned int InnerHeight();
	virtual unsigned int InnerWidth();

	virtual void DrawBorder(short int x, short int y);
	virtual void DrawContent(short int x, short int y) {}


public:
	Widget(shared_ptr<string> title, BorderSize border);

	unsigned int Height();
	unsigned int Width();

	void Draw(short int x, short int y);

	virtual void Focus() {}
};


class Interface
{
private:
	shared_ptr<Widget> _widget;
public:
	Interface(shared_ptr<Widget> widget);

	void Draw();
};

class Container : public Widget {
public:
	Container(shared_ptr<string> title, BorderSize border) : Widget(title, border) {}
};

class Row : public Container {
private:
	shared_ptr<vector<shared_ptr<Widget>>> _widgets;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Row(shared_ptr<vector<shared_ptr<Widget>>> widgets, shared_ptr<string> title, BorderSize border);
};

class Column : public Container {
private:
	shared_ptr<vector<shared_ptr<Widget>>> _widgets;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Column(shared_ptr<vector<shared_ptr<Widget>>> widgets, shared_ptr<string> title, BorderSize border);

};


class TextBox : public Widget {
private:
	shared_ptr<string> _text;

	unsigned int _width;
	unsigned int _height;
	int _x = 0;
	int _y = 0;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	TextBox(shared_ptr<string> text, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height);
	shared_ptr<string> Text();

	void Focus();
};

class Fader : public Widget {
private:
	shared_ptr<double> _value;

	unsigned int _height;
	unsigned int _width;
	
	double _max;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);

public:
	Fader(shared_ptr<double> value, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height, double max);
	shared_ptr<double> Value();
};