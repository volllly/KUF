#pragma once
#include <windows.h>
#include <optional>
#include <functional>
#include <numeric>

#include "Protocol.h"

using namespace std;

enum class Navigation {
	Left,
	Right,
	Up,
	Down,
	In,
	Out
};

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
	Widget(string&& title, BorderSize border) : Widget(make_shared<string>(title), border) {}

	unsigned int Height();
	unsigned int Width();

	void Draw(short int x, short int y);
	virtual void Clear(short int x, short int y);
	virtual void Navigate(Navigation direction) {}
};


class Interface : public enable_shared_from_this<Interface>
{
private:
	shared_ptr<Widget> _widget;
	shared_ptr<Widget> _current;

public:
	Interface(shared_ptr<Widget> widget);
	template <class T>
	Interface(T&& widget) : Interface((shared_ptr<Widget>)make_shared<T>(widget)) {}

	void Draw(); //TODO: make private

	void Run();
};

class Container : public Widget {
public:
	Container(shared_ptr<string> title, BorderSize border) : Widget(title, border) {}
	Container(string&& title, BorderSize border) : Container(make_shared<string>(title), border) {}

	virtual void Clear(short int x, short int y);
};

class Row : public Container {
private:
	shared_ptr<vector<shared_ptr<Widget>>> _widgets;
	unsigned int _active;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Row(shared_ptr<vector<shared_ptr<Widget>>> widgets, shared_ptr<string> title, BorderSize border);

	Row(vector<shared_ptr<Widget>>&& widgets, shared_ptr<string> title, BorderSize border) : Row(make_shared<vector<shared_ptr<Widget>>>(widgets), title, border) {}
	Row(shared_ptr<vector<shared_ptr<Widget>>> widgets, string&& title, BorderSize border) : Row(widgets, make_shared<string>(title), border) {}

	Row(vector<shared_ptr<Widget>>&& widgets, string&& title, BorderSize border) : Row(move(widgets), make_shared<string>(title), border) {}
	
	void Navigate(Navigation direction);
};

class Column : public Container {
private:
	shared_ptr<vector<shared_ptr<Widget>>> _widgets;
	unsigned int _active;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	Column(shared_ptr<vector<shared_ptr<Widget>>> widgets, shared_ptr<string> title, BorderSize border);

	Column(vector<shared_ptr<Widget>>&& widgets, shared_ptr<string> title, BorderSize border) : Column(make_shared<vector<shared_ptr<Widget>>>(widgets), title, border) {}
	Column(shared_ptr<vector<shared_ptr<Widget>>> widgets, string&& title, BorderSize border) : Column(widgets, make_shared<string>(title), border) {}

	Column(vector<shared_ptr<Widget>>&& widgets, string title, BorderSize border) : Column(move(widgets), make_shared<string>(title), border) {}

	void Navigate(Navigation direction);
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

	TextBox(string&& text, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height) : TextBox(make_shared<string>(text), title, border, width, height) {}
	TextBox(shared_ptr<string> text, string&& title, BorderSize border, unsigned int width, unsigned int height) : TextBox(text, make_shared<string>(title), border, width, height) {}

	TextBox(string&& text, string&& title, BorderSize border, unsigned int width, unsigned int height) : TextBox(move(text), make_shared<string>(title), border, width, height) {}


	shared_ptr<string> Text();

	void Navigate(Navigation direction);
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

	Fader(double&& value, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height, double max) : Fader(make_shared<double>(value), title, border, width, height, max) {}
	Fader(shared_ptr<double> value, string&& title, BorderSize border, unsigned int width, unsigned int height, double max) : Fader(value, make_shared<string>(move(title)), border, width, height, max) {}
	
	Fader(double&& value, string&& title, BorderSize border, unsigned int width, unsigned int height, double max) : Fader(move(value), make_shared<string>(move(title)), border, width, height, max) {}

	//void Navigate(Navigation direction, shared_ptr<Interface> interface);

	shared_ptr<double> Value();
};