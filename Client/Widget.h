#pragma once
#include <windows.h>
#include <optional>
#include <functional>
#include <numeric>


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
	virtual void Navigate(Navigation direction, shared_ptr<Interface> ifce) {}
};
