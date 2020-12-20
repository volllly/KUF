#include "Interface.h"
#include <sstream>
#include <iomanip> 


#include <io.h>
#include <fcntl.h>
#include <conio.h>

Widget::Widget(shared_ptr<string> title, BorderSize border) {
	_border = border;
	_title = title;
}


void Widget::DrawBorder(short int x, short int y) {
	auto border = _border;

	if (GetActivated()) {
		border = BorderSize::Dotted;
	}

	if (border != BorderSize::None) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y });
		auto symbols = BorderAttributes.at(border);

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

	if (_title || GetSelected()) {
		string title;
		if (_title) {
			title = *_title;
		}
		else {
			title = "";
		}

		if (GetSelected()) {
			title = "*" + title;
		}

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + 1, y });
		cout << title;
	}
}

void Widget::Clear(short int x, short int y) {
	for (unsigned int j = 0; j < InnerHeight(); j++) {
		for (unsigned int i = 0; i < InnerWidth(); i++) {
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + (short)i, y + (short)j });
			cout << ' ';
		}
	}
}

void Widget::Draw(short int x, short int y) {
	//Clear(x, y);
	DrawBorder(x, y);
	auto marginX = (GetActivated() || _border != BorderSize::None) ? 1 : 0;
	auto marginY = (GetActivated() || _border != BorderSize::None) || (_title || GetSelected()) ? 1 : 0;
	DrawContent(x + marginX, y + marginY);
}
unsigned int Widget::InnerWidth() {
	return 0;
}
unsigned int Widget::InnerHeight() {
	return 0;
}

unsigned int Widget::Width() {
	return InnerWidth() + (GetActivated() || _border != BorderSize::None ? 2 : 0);
}

unsigned int Widget::Height() {
	auto margin = _title || GetSelected() ? 1 : 0;
	if(GetActivated() || _border != BorderSize::None) {
		margin = 2;
	}

	return InnerHeight() + margin;
}

Interface::Interface(shared_ptr<Widget> widget) {
	_widget = widget;
}

void Interface::Run() {
	_current = {};

	StepIn(_widget);
	Draw();

	char input;
	while (input = _getch()) {
		switch (input)
		{
		case 'h':
			_current.back()->Navigate(Navigation::Left, shared_from_this());
			break;
		case 'l':
			_current.back()->Navigate(Navigation::Right, shared_from_this());
			break;
		case 'j':
			_current.back()->Navigate(Navigation::Down, shared_from_this());
			break;
		case 'k':
			_current.back()->Navigate(Navigation::Up, shared_from_this());
			break;
		case 'i':
			_current.back()->Navigate(Navigation::In, shared_from_this());
			break;
		case 'o':
			_current.back()->Navigate(Navigation::Out, shared_from_this());
			break;
		case 'q':
			return;
		default:
			break;
		}
		Draw();
	}
}

void Interface::StepIn(shared_ptr<Widget> in) {
	if (_current.size() > 0) {
		_current.back()->SetActivated(false);
	}
	_current.push_back(in);
	in->SetActivated(true);
	in->Navigate(Navigation::Activate, shared_from_this());
}

void Interface::StepOut() {
	if (_current.size() > 1) {
		_current.back()->SetActivated(false);
		_current.pop_back();
		_current.back()->SetActivated(true);
		_current.back()->Navigate(Navigation::Deactivate, shared_from_this());
	}
}

void Interface::Draw() {
	cout.sync_with_stdio(false);
	system("cls");
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	CONSOLE_CURSOR_INFO cursor {
		1,
		false
	};

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);

	CONSOLE_SCREEN_BUFFER_INFO info;
	unsigned short int x = 0, y = 0;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
		x = info.srWindow.Left;
		y = info.srWindow.Top;
	}

	_widget->Draw(x, y);
	cout.sync_with_stdio(true);
	cout.flush();
}


void Container::Clear(short int x, short int y) {
	if (!(_title || GetSelected())) { return; }

	for (unsigned int j = 0; j < InnerHeight(); j++) {
		for (unsigned int i = 0; i < InnerWidth(); i++) {
			if ((i > 0) && (i < InnerWidth()) && (j > 0) && (j < InnerHeight())) { continue; }
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + (short)i, y + (short)j });
			cout << ' ';
		}

		if (_border != BorderSize::None) { return; }
	}
}

void Row::Updated() {
	_active = 0;

	for (unsigned int i = 0; i < _widgets->size(); i++) {
		_widgets->at(i)->SetSelected(i == _active);
	}
}

Row::Row(shared_ptr<vector<shared_ptr<Widget>>> widgets, shared_ptr<string> title, BorderSize border) : Container(title, border) {
	_widgets = widgets;
	_active = 0;
}

void Row::Navigate(Navigation direction, shared_ptr<Interface> ifce) {
	switch (direction) {
	case Navigation::Left:
		_widgets->at(_active)->SetSelected(false);
		if (_active == 0) {
			_active = _widgets->size();
		}
		_active--;
		_widgets->at(_active)->SetSelected(true);
		break;
	case Navigation::Right:
		_widgets->at(_active)->SetSelected(false);
		_active++;
		if (_active >= _widgets->size()) {
			_active = 0;
		}
		_widgets->at(_active)->SetSelected(true);
		break;
	case Navigation::In:
		_widgets->at(_active)->SetSelected(false);
		ifce->StepIn(_widgets->at(_active));
		break;
	case Navigation::Activate:
		if (_widgets->size() == 1) {
			_widgets->at(_active)->SetSelected(false);
			ifce->StepIn(_widgets->at(0));
		}
		break;
	case Navigation::Out:
		_widgets->at(_active)->SetSelected(false);
		ifce->StepOut();
		break;
	}
}

void Row::SetActivated(bool activated) {
	_activated = activated;
	//_active = 0;
	if (activated) {
		_widgets->at(_active)->SetSelected(true);
	}
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
	_active = 0;
}

void Column::Navigate(Navigation direction, shared_ptr<Interface> ifce) {
	switch (direction) {
	case Navigation::Up:
		_widgets->at(_active)->SetSelected(false);
		if (_active == 0) {
			_active = _widgets->size();
		}
		_active--;
		_widgets->at(_active)->SetSelected(true);
		break;
	case Navigation::Down:
		_widgets->at(_active)->SetSelected(false);
		_active++;
		if (_active >= _widgets->size()) {
			_active = 0;
		}
		_widgets->at(_active)->SetSelected(true);
		break;
	case Navigation::In:
		_widgets->at(_active)->SetSelected(false);
		ifce->StepIn(_widgets->at(_active));
		break;
	case Navigation::Activate:
		if (_widgets->size() == 1) {
			_widgets->at(_active)->SetSelected(false);
			ifce->StepIn(_widgets->at(0));
		}
		break;
	case Navigation::Out:
		_widgets->at(_active)->SetSelected(false);
		ifce->StepOut();
		break;
	}
}

void Column::SetActivated(bool activated) {
	_activated = false;
	//_active = 0;

	if(activated) {
		_widgets->at(_active)->SetSelected(true);
	}
}

void Column::Updated() {
	for (unsigned int i = 0; i < _widgets->size(); i++) {
		_widgets->at(i)->SetSelected(_widgets->at(i)->GetActivated());
		if (_widgets->at(i)->GetActivated()) { _active = i; }
	}
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


TextBox::TextBox(shared_ptr<string> text, shared_ptr<string> title, BorderSize border, unsigned int width, unsigned int height, bool readonly) : Widget(title, border) {
	_width = width;
	_height = height;
	_text = text;
	_readonly = readonly;
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

	vector<string> lines{};

	while (getline(iss, line))
	{
		while (line.length() > InnerWidth()) {
			lines.push_back(line.substr(0, InnerWidth()));
			line = line.substr(InnerWidth());
		}

		lines.push_back(line);
	}
	
	for (unsigned int i = (unsigned)max((signed)lines.size() - (signed)InnerHeight(), 0); i < (unsigned short)lines.size(); i++) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y + (short)i - max((unsigned short int)lines.size() - (unsigned short int)InnerHeight(), 0) });
		cout << lines[i];
	}
}

shared_ptr<string> TextBox::Text() {
	return _text;
}

void TextBox::Navigate(Navigation direction, shared_ptr<Interface> ifce) {
	if (direction == Navigation::In || direction == Navigation::Activate) {
		ifce->Draw();
		if (!_readonly) {
			CONSOLE_CURSOR_INFO cursor{
				1,
				true
			};

			SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);

			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ (short)_x, (short)_y });

			string line;
			getline(cin, line);

			*_text = line;
			if (_callback) {
				_callback(shared_from_this());
			}
		}
		ifce->StepOut();
	}
}

void TextBox::OnChange(function<void(shared_ptr<TextBox>)> callback) {
	_callback = callback;
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

	if (value >= 0) {
		ostringstream fmts;
		fmts << setprecision(InnerWidth() - 1) << value;
		string fmt = fmts.str();

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + ((short)InnerWidth() - (short)fmt.length()) / 2, y + (short)InnerHeight() - 1 });

		cout << fmt;
	}
	else {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ center - (width - 1) / 2, y + (short)InnerHeight() - 1 });
		for (short j = 0; j < width; j++) {
			cout << '-';
		}
	}


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


void Fader::Navigate(Navigation direction, shared_ptr<Interface> ifce) {
	switch (direction) {
	case Navigation::Up:
		*_value += 10;
		if (*_value > _max) { *_value = _max;  }
		break;
	case Navigation::Down:
		if (*_value < 10) { *_value = 0; } else { *_value -= 10; }
		break;
	case Navigation::Out:
		ifce->StepOut();
		break;
	}

	if (_callback && (direction == Navigation::Up || direction == Navigation::Down)) {
		_callback(shared_from_this());
	}
}

void Fader::OnChange(function<void(shared_ptr<Fader>)> callback) {
	_callback = callback;
}