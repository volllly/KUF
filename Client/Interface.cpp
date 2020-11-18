#include "Interface.h"
#include <sstream>
#include <iomanip> 


#include <io.h>
#include <fcntl.h>

Widget::Widget(shared_ptr<string> title, BorderSize border) {
	_border = border;
	_title = title;
}


void Widget::DrawBorder(short int x, short int y) {
	if (_border != BorderSize::None) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y });
		BorderSymbols symbols = BorderAttributes.at(_border);

		cout << symbols.tl;
		for (unsigned int i = 0; i < InnerWidth(); i++) {
			cout << symbols.h;
		}
		cout << symbols.tr;

		for (unsigned int j = 0; j < InnerHeight(); j++) {
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + 1 + (short)j });
			cout << symbols.v;
			for (unsigned int i = 0; i < InnerWidth(); i++) {
				cout << ' ';
			}
			cout << symbols.v;
		}

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + 1 + (short)InnerHeight() });
		cout << symbols.bl;
		for (unsigned int i = 0; i < InnerWidth(); i++) {
			cout << symbols.h;
		}
		cout << symbols.br;
	}

	if (_title) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + 1, y });
		cout << *_title;
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

Interface::Interface(shared_ptr<Widget> widget) {
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
	_widget->Draw(x, y);
}

Row::Row(shared_ptr<vector<shared_ptr<Widget>>> widgets, shared_ptr<string> title, BorderSize border) : Container(title, border) {
	_widgets = widgets;
}

void Row::DrawContent(short int x, short int y) {
	unsigned int width = x;
	for (auto& widget : *_widgets) {
		widget->Draw(width, y);
		width += widget->Width();
	}
}

unsigned int Row::InnerWidth() {
	unsigned int width = 0;
	for (auto& widget : *_widgets) {
		width += widget->Width();
	}

	return width;
}

unsigned int Row::InnerHeight() {
	unsigned int height = 0;
	for (auto& widget : *_widgets) {
		height = max(widget->Height(), height);
	}

	return height;
}



Column::Column(shared_ptr<vector<shared_ptr<Widget>>> widgets, shared_ptr<string> title, BorderSize border) : Container(title, border) {
	_widgets = widgets;
}

void Column::DrawContent(short int x, short int y) {
	unsigned int height = y;
	for (auto& widget : *_widgets) {
		widget->Draw(x, height);
		height += widget->Height();
	}
}

unsigned int Column::InnerHeight() {
	unsigned int height = 0;
	for (auto& widget : *_widgets) {
		height += widget->Height();
	}

	return height;
}

unsigned int Column::InnerWidth() {
	unsigned int width = 0;
	for (auto& widget : *_widgets) {
		width = max(widget->Width(), width);
	}

	return width;
}


TextBox::TextBox(shared_ptr<string> text, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height) : Widget(title, border) {
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

	string text = *_text;

	istringstream iss(text);
	string line;

	vector<string> lines = vector<string>{};

	while (getline(iss, line))
	{
		while (line.length() > InnerWidth()) {
			lines.push_back(line.substr(0, InnerWidth()));
			line = line.substr(InnerWidth());
		}

		lines.push_back(line);
	}
	
	for (unsigned short int i = (unsigned)max((signed)lines.size() - (signed)InnerHeight(), 0); i < (short)lines.size(); i++) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + i - max((unsigned short int)lines.size() - (unsigned short int)InnerHeight(), 0) });
		cout << lines[i];
	}
}

shared_ptr<string> TextBox::Text() {
	return _text;
}

void TextBox::Focus() {
	CONSOLE_CURSOR_INFO cursor = {
		1,
		true
	};

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ (short)_x, (short)_y });

	string line;
	getline(cin, line);

	*_text = line;

}

Fader::Fader(shared_ptr<double> value, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height, double max) : Widget(title, border) {
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

const string FaderChars[] = {
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
	double value = *_value;

	short int center = x + ((short)InnerWidth() - 1) / 2;
	short width = (short)InnerWidth() - 2;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y + (short)InnerHeight() - 2 });
	for (short j = 0; j < width; j++) {
		cout << '=';
	}

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y });
	for (short j = 0; j < width; j++) {
		cout << '=';
	}

	ostringstream fmts;
	fmts << setprecision(InnerWidth() - 1) << value;
	string fmt = fmts.str();

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + ((short)InnerWidth() - (short)fmt.length()) / 2, y + (short)InnerHeight() - 1 });

	cout << fmt;

	unsigned int bar = (unsigned int)(min(value / _max, 1) * 8 * (InnerHeight() - 3));

	for (unsigned int i = 0; i < bar; i++) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y + (short)InnerHeight() - 3 - ((unsigned short)i / 8) });
		for (short j = 0; j < width; j++) {
			cout << FaderChars[i % 8];
		}
	}
}

shared_ptr<double> Fader::Value() {
	return _value;
}
