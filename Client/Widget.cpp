#include "Widget.h"
#include "Interface.h"

using namespace std;

Widget::Widget(shared_ptr<string> title, BorderSize border) {
	_border = border;
	_title = title;
}


void Widget::DrawBorder(short int x, short int y) {
	if (_border != BorderSize::None) {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x, y });
		auto symbols = BorderAttributes.at(_border);

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

void Widget::Clear(short int x, short int y) {
	for (unsigned int j = 0; j < InnerHeight(); j++) {
		for (unsigned int i = 0; i < InnerWidth(); i++) {
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ x + (short)i, y + (short)j });
			cout << ' ';
		}
	}
}

void Widget::Draw(short int x, short int y) {
	Clear(x, y);
	DrawBorder(x, y);
	auto marginX = (_border != BorderSize::None ? 1 : 0);
	auto marginY = (_border != BorderSize::None || _title ? 1 : 0);
	DrawContent(x + marginX, y + marginY);
}
unsigned int Widget::InnerWidth() {
	return 0;
}
unsigned int Widget::InnerHeight() {
	return 0;
}

unsigned int Widget::Width() {
	return InnerWidth() + (_border != BorderSize::None ? 2 : 0);
}

unsigned int Widget::Height() {
	auto margin = _title ? 1 : 0;
	if (_border != BorderSize::None) {
		margin = 2;
	}

	return InnerHeight() + margin;
}
