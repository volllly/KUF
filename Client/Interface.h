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
	Single,
	Double
};

const struct BorderSymbols {
	char tl;
	char tr;
	char h;
	char v;
	char bl;
	char br;
};

const std::map<BorderSize, BorderSymbols> BorderAttributes = {
	{BorderSize::Single, BorderSymbols {
		(char)0xDA,
		(char)0xBF,
		(char)0xC4,
		(char)0xB3,
		(char)0xC0,
		(char)0xD9,
	}},
	{BorderSize::Double, BorderSymbols {
		(char)0xC9,
		(char)0xBB,
		(char)0xCD,
		(char)0xBA,
		(char)0xC8,
		(char)0xBC,
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

	unsigned int InnerHeight();
	unsigned int InnerWidth();
	void DrawContent(short int x, short int y);
public:
	TextBox(::Binding binding, std::optional<std::string> title, BorderSize border, unsigned int width, unsigned int height);
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