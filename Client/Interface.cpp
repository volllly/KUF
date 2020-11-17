#include "Interface.h"
#include <sstream>
#include <iomanip> 


#include <io.h>
#include <fcntl.h>

Widget::Widget(std::shared_ptr<std::string> title, BorderSize border) {
	_border = border;
	_title = title;
}


void Widget::DrawBorder(short int x, short int y) {
	if (_border != BorderSize::None) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y });
		BorderSymbols symbols = BorderAttributes.at(_border);

		std::cout << symbols.tl;
		for (unsigned int i = 0; i < InnerWidth(); i++) {
			std::cout << symbols.h;
		}
		std::cout << symbols.tr;

		for (unsigned int j = 0; j < InnerHeight(); j++) {
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + 1 + (short)j });
			std::cout << symbols.v;
			for (unsigned int i = 0; i < InnerWidth(); i++) {
				std::cout << ' ';
			}
			std::cout << symbols.v;
		}

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + 1 + (short)InnerHeight() });
		std::cout << symbols.bl;
		for (unsigned int i = 0; i < InnerWidth(); i++) {
			std::cout << symbols.h;
		}
		std::cout << symbols.br;
	}

	if (_title) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + 1, y });
		std::cout << *_title.get();
	}
}

void Widget::Draw(short int x, short int y) {
	DrawBorder(x, y);
	auto margin = (_border != BorderSize::None || _title ? 1 : 0);
	DrawContent(x + margin, y + margin);
}
unsigned int Widget::InnerWidth() {
	return 0;
}
unsigned int Widget::InnerHeight() {
	return 0;
}

unsigned int Widget::Width() {
	return InnerWidth() + (_border != BorderSize::None || _title ? 2 : 0);
}

unsigned int Widget::Height() {
	return InnerHeight() + (_border != BorderSize::None || _title ? 2 : 0);
}

Interface::Interface(std::shared_ptr<Widget> widget) {
	_widget = widget;
}

void Interface::Draw() {
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	CONSOLE_SCREEN_BUFFER_INFO info;
	unsigned short int x = 0, y = 0;

	DWORD written;

	CONSOLE_CURSOR_INFO cursor = {
		1,
		false
	};

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);

	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
		FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', info.dwSize.X * info.dwSize.Y, COORD { 0, 0 }, &written);
		x = info.srWindow.Left;
		y = info.srWindow.Top;
	}
	_widget.get()->Draw(x, y);
}

Row::Row(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets, std::shared_ptr<std::string> title, BorderSize border) : Container(title, border) {
	_widgets = widgets;
}

void Row::DrawContent(short int x, short int y) {
	unsigned int width = x;
	for (auto& widget : *_widgets.get()) {
		widget.get()->Draw(width, y);
		width += widget.get()->Width();
	}
}

unsigned int Row::InnerWidth() {
	unsigned int width = 0;
	for (auto& widget : *_widgets) {
		width += widget.get()->Width();
	}

	return width;
}

unsigned int Row::InnerHeight() {
	unsigned int height = 0;
	for (auto& widget : *_widgets.get()) {
		height = max(widget.get()->Height(), height);
	}

	return height;
}



Column::Column(std::shared_ptr<std::vector<std::shared_ptr<Widget>>> widgets, std::shared_ptr<std::string> title, BorderSize border) : Container(title, border) {
	_widgets = widgets;
}

void Column::DrawContent(short int x, short int y) {
	unsigned int height = y;
	for (auto& widget : *_widgets.get()) {
		widget.get()->Draw(x, height);
		height += widget.get()->Height();
	}
}

unsigned int Column::InnerHeight() {
	unsigned int height = 0;
	for (auto& widget : *_widgets.get()) {
		height += widget.get()->Height();
	}

	return height;
}

unsigned int Column::InnerWidth() {
	unsigned int width = 0;
	for (auto& widget : *_widgets.get()) {
		width = max(widget.get()->Width(), width);
	}

	return width;
}


TextBox::TextBox(std::shared_ptr<std::string> text, std::shared_ptr<std::string> title, BorderSize border, unsigned int width, unsigned int height) : Widget(title, border) {
	_width = width;
	_height = height;
	_text = text;
}


unsigned int TextBox::InnerHeight() {
	return _height;
}
unsigned int TextBox::InnerWidth() {
	return _width;
}

void TextBox::DrawContent(short int x, short int y) {
	_x = x;
	_y = y;

	std::string text = *_text.get();

	std::istringstream iss(text);
	std::string line;

	std::vector<std::string> lines = std::vector<std::string>{};

	while (std::getline(iss, line))
	{
		while (line.length() > InnerWidth()) {
			lines.push_back(line.substr(0, InnerWidth()));
			line = line.substr(InnerWidth());
		}

		lines.push_back(line);
	}
	
	for (unsigned short int i = (unsigned)max((signed)lines.size() - (signed)InnerHeight(), 0); i < (short)lines.size(); i++) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + i - max((unsigned short int)lines.size() - (unsigned short int)InnerHeight(), 0) });
		std::cout << lines[i];
	}
}

std::shared_ptr<std::string> TextBox::Text() {
	return _text;
}

void TextBox::Focus() {
	CONSOLE_CURSOR_INFO cursor = {
		1,
		true
	};

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ (short)_x, (short)_y });

	std::string line;
	std::getline(std::cin, line);

	*_text.get() = line;

}

Fader::Fader(std::shared_ptr<double> value, std::shared_ptr<std::string> title, BorderSize border, unsigned int width, unsigned int height, double max) : Widget(title, border) {
	_width = width;
	_height = height;
	_max = max;
	_title = title;
	_value = value;
}

unsigned int Fader::InnerHeight() {
	return _height + 1;
}
unsigned int Fader::InnerWidth() {
	return (unsigned int)max(_width, abs(log10(_max)) + 2);
}

const std::string FaderChars[] = {
	u8"▁",
	u8"▂",
	u8"▃",
	u8"▄",
	u8"▅",
	u8"▆",
	u8"▇",
	u8"█",
};

void Fader::DrawContent(short int x, short int y) {
	double value = *_value.get();

	short int center = x + ((short)InnerWidth() - 1) / 2;
	short width = (short)InnerWidth() - 2;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y + (short)InnerHeight() - 2 });
	for (short j = 0; j < width; j++) {
		std::cout << '=';
	}

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y });
	for (short j = 0; j < width; j++) {
		std::cout << '=';
	}

	std::ostringstream fmts;
	fmts << std::setprecision(InnerWidth() - 1) << value;
	std::string fmt = fmts.str();

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + ((short)InnerWidth() - (short)fmt.length()) / 2, y + (short)InnerHeight() - 1 });

	std::cout << fmt;

	unsigned int bar = (unsigned int)(min(value / _max, 1) * 8 * (InnerHeight() - 3));

	for (unsigned int i = 0; i < bar; i++) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y + (short)InnerHeight() - 3 - ((unsigned short)i / 8) });
		for (short j = 0; j < width; j++) {
			std::cout << FaderChars[i % 8];
		}
	}
}

std::shared_ptr<double> Fader::Value() {
	return _value;
}
