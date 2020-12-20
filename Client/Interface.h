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
	Out,
	Activate,
	Deactivate
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
class Interface;

class Widget {
protected:
	shared_ptr<string> _title;
	BorderSize _border;
	bool _activated = false;
	bool _selected = false;

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
	virtual void Navigate(Navigation direction, shared_ptr<Interface> ifce) {}

	bool GetActivated() { return _activated; }
	virtual void SetActivated(bool activated) { _activated = activated; }

	bool GetSelected() { return _selected; }
	virtual void SetSelected(bool selected) { _selected = selected; }
};


class Interface : public enable_shared_from_this<Interface>
{
private:
	shared_ptr<Widget> _widget;
	vector<shared_ptr<Widget>> _current;

public:
	Interface(shared_ptr<Widget> widget);
	template <class T>
	Interface(T&& widget) : Interface((shared_ptr<Widget>)make_shared<T>(widget)) {}

	void Draw(); //TODO: make private

	void Run();

	void StepIn(shared_ptr<Widget>);
	void StepOut();
};

class Container : public Widget {
public:
	Container(shared_ptr<string> title, BorderSize border) : Widget(title, border) {}
	Container(string&& title, BorderSize border) : Container(make_shared<string>(title), border) {}

	virtual void Clear(short int x, short int y);
	virtual void Updated() {};
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
	
	void Navigate(Navigation direction, shared_ptr<Interface> ifce);
	void SetActivated(bool activated);

	void Updated();

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

	void Navigate(Navigation direction, shared_ptr<Interface> ifce);
	void SetActivated(bool activated);

	void Updated();
};


class TextBox : public Widget, public enable_shared_from_this<TextBox> {
private:
	shared_ptr<string> _text;
	function<void(shared_ptr<TextBox>)> _callback;
	bool _readonly;

	unsigned int _width;
	unsigned int _height;
	int _x = 0;
	int _y = 0;

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	TextBox(shared_ptr<string> text, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height, bool readonly);

	TextBox(string&& text, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height, bool readonly) : TextBox(make_shared<string>(text), title, border, width, height, readonly) {}
	TextBox(shared_ptr<string> text, string&& title, BorderSize border, unsigned int width, unsigned int height, bool readonly) : TextBox(text, make_shared<string>(title), border, width, height, readonly) {}

	TextBox(string&& text, string&& title, BorderSize border, unsigned int width, unsigned int height, bool readonly) : TextBox(move(text), make_shared<string>(title), border, width, height, readonly) {}


	shared_ptr<string> Text();

	void Navigate(Navigation direction, shared_ptr<Interface> ifce);

	void OnChange(function<void(shared_ptr<TextBox>)> callback);
};

class Fader : public Widget, public enable_shared_from_this<Fader> {
private:
	shared_ptr<double> _value;
	function<void(shared_ptr<Fader>)> _callback;

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

	void Navigate(Navigation direction, shared_ptr<Interface> ifce);

	void OnChange(function<void(shared_ptr<Fader>)> callback);

	shared_ptr<double> Value();
};