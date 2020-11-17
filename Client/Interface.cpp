#include "Interface.h"
#include <sstream>
#include <iomanip> 


Binding::Binding(std::shared_ptr<void> data, std::function<void(std::shared_ptr<void>)> handler) {
	_data = data;
	_handler = handler;
}

std::shared_ptr<void> Binding::Get() {
	return _data;
}

void Binding::Update() {
	_handler(_data);
}

Widget::Widget(std::optional<::Binding> binding, std::optional<std::string> title, BorderSize border) {
	_binding = binding;
	_border = border;
	_title = title;
}

Binding* Widget::Binding() {
	return &_binding.value();
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

	if (_title.has_value()) {
		auto tmp = _title.value();
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + 1, y });
		std::cout << _title.value();
	}
}

void Widget::Draw(short int x, short int y) {
	DrawBorder(x, y);
	auto margin = (_border != BorderSize::None || _title.has_value() ? 1 : 0);
	DrawContent(x + margin, y + margin);
}
unsigned int Widget::InnerWidth() {
	return 0;
}
unsigned int Widget::InnerHeight() {
	return 0;
}

unsigned int Widget::Width() {
	return InnerWidth() + (_border != BorderSize::None || _title.has_value() ? 2 : 0);
}

unsigned int Widget::Height() {
	return InnerHeight() + (_border != BorderSize::None || _title.has_value() ? 2 : 0);
}

Interface::Interface(std::shared_ptr<Widget> widget) {
	_widget = widget;
}

void Interface::Input(INPUT_RECORD input) {
	_widget.get()->Input(input);
}

void Interface::Draw() {
	CONSOLE_SCREEN_BUFFER_INFO info;
	unsigned short int x = 0, y = 0;

	DWORD written;

	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
		FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ', info.dwSize.X * info.dwSize.Y, COORD { 0, 0 }, &written);
		x = info.srWindow.Left;
		y = info.srWindow.Top;
	}
	_widget.get()->Draw(x, y);
}

Row::Row(std::vector<std::shared_ptr<Widget>> widgets, std::optional<std::string> title, BorderSize border) : Container(title, border) {
	_widgets = widgets;
}

void Row::DrawContent(short int x, short int y) {
	unsigned int width = x;
	for (auto& widget : _widgets) {
		widget.get()->Draw(width, y);
		width += widget.get()->Width();
	}
}

unsigned int Row::InnerWidth() {
	unsigned int width = 0;
	for (auto& widget : _widgets) {
		width += widget.get()->Width();
	}

	return width;
}

unsigned int Row::InnerHeight() {
	unsigned int height = 0;
	for (auto& widget : _widgets) {
		height = max(widget.get()->Height(), height);
	}

	return height;
}



Column::Column(std::vector<std::shared_ptr<Widget>> widgets, std::optional<std::string> title, BorderSize border) : Container(title, border) {
	_widgets = widgets;
}

void Column::DrawContent(short int x, short int y) {
	unsigned int height = y;
	for (auto& widget : _widgets) {
		widget.get()->Draw(x, height);
		height += widget.get()->Height();
	}
}

unsigned int Column::InnerHeight() {
	unsigned int height = 0;
	for (auto& widget : _widgets) {
		height += widget.get()->Height();
	}

	return height;
}

unsigned int Column::InnerWidth() {
	unsigned int width = 0;
	for (auto& widget : _widgets) {
		width = max(widget.get()->Width(), width);
	}

	return width;
}


TextBox::TextBox(::Binding binding, std::optional<std::string> title, BorderSize border, unsigned int width, unsigned int height) : Widget(binding, title, border) {
	_width = width;
	_height = height;
}


unsigned int TextBox::InnerHeight() {
	return _height;
}
unsigned int TextBox::InnerWidth() {
	return _width;
}

void TextBox::DrawContent(short int x, short int y) {
	std::string text = *(std::string*)_binding.value().Get().get();

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
	
	for (unsigned short int i = (unsigned)max((signed)lines.size() - (signed)InnerHeight(), 0); i < lines.size(); i++) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + i - max((unsigned short int)lines.size() - (unsigned short int)InnerHeight(), 0) });
		std::cout << lines[i];
	}
}



Fader::Fader(::Binding binding, std::optional<std::string> title, BorderSize border, unsigned int width, unsigned int height, double max) : Widget(binding, title, border) {
	_width = width;
	_height = height;
	_max = max;
	_title = title;
}

unsigned int Fader::InnerHeight() {
	return _height + 1;
}
unsigned int Fader::InnerWidth() {
	return (unsigned int)max(_width, abs(log10(_max)) + 2);
}

const char FaderChars[] = {
	0xB0, //░ 
	0xB1, //▒
	0xB2, //▓
	0xDB, //█
};

void Fader::DrawContent(short int x, short int y) {
	double value = *(double*)_binding.value().Get().get();

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

	unsigned int bar = (unsigned int)(min(value / _max, 1) * 4 * (InnerHeight() - 3));

	for (unsigned int i = 0; i < bar; i++) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y + (short)InnerHeight() - 3 - ((unsigned short)i / 4) });
		for (short j = 0; j < width; j++) {
			std::cout << FaderChars[i % 4];
		}
	}
}