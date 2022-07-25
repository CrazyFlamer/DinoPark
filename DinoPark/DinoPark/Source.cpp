#include <Windows.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <ctime>
#include <unordered_map>

using namespace std;

class Point {
	int _x, _y;

public:
	Point() {
		_x = 0;
		_y = 0;
	}

	Point(int x, int y) {
		_x = x;
		_y = y;
	}

	int getX() { return _x; }
	int getY() { return _y; }

};

// Отсюда вы можете брать цвета и здесь же создавать свои
class ConsoleColor {
public:
	static COLORREF red() { return RGB(255, 0, 0); }
	static COLORREF blue() { return RGB(0, 0, 255); }
	static COLORREF gray() { return RGB(200, 200, 200); }
	static COLORREF white() { return RGB(255, 255, 255); }
	static COLORREF green() { return RGB(0, 255, 0); }
	static COLORREF black() { return RGB(0, 0, 0); }
	static COLORREF brown() { return RGB(80, 20, 25); }
	static COLORREF yellow() { return RGB(255, 255, 0); }
	static COLORREF orange() { return RGB(255, 127, 80); }
	static COLORREF darkgreen() { return RGB(0, 100, 0); }
	static COLORREF lighgreen() { return RGB(124, 252, 0); }
	static COLORREF violent() { return RGB(75, 0, 130); }
};

// Это класс, который нужен для рисования. В нем все готово. Ничего менять не нужно. Нужно только пользоваться
class ConsoleDrawer {
	HANDLE _conHandle;
	HWND _hwnd;
	HDC _hdc;
	PAINTSTRUCT _ps;
	unordered_map<COLORREF, HGDIOBJ> _bm;
	unordered_map<COLORREF, HGDIOBJ> _pm;
	HGDIOBJ _curentBrush;
	HGDIOBJ _curentPen;

	int _width;
	int _height;
	bool _isWork = true;

	void selectBrush(COLORREF color, bool filled = false) {
		if (!filled) {
			SelectObject(_hdc, CreateSolidBrush(NULL_BRUSH));
			return;
		}
		if (_bm.find(color) == _bm.end())
			_bm.insert({ color, CreateSolidBrush(color) });

		if (_curentBrush != _bm[color]) {
			_curentBrush = _bm[color];
			SelectObject(_hdc, _curentBrush);
		}
	}

	void selectPen(COLORREF color) {
		if (_pm.find(color) == _pm.end())
			_pm.insert({ color, CreatePen(PS_SOLID, 1, color) });
		if (_curentPen != _pm[color]) {
			_curentPen = _pm[color];
			SelectObject(_hdc, _curentPen);
		}
	}

public:
	ConsoleDrawer() {
		setlocale(LC_ALL, "");
		_conHandle = GetStdHandle(STD_INPUT_HANDLE);
		int t = sizeof(HGDIOBJ);
		_hwnd = GetConsoleWindow();
		RECT rc;
		GetClientRect(_hwnd, &rc);
		_width = rc.right;
		_height = rc.bottom;
		_hdc = GetDC(_hwnd);
	}

	void clearScreen() {
		system("cls");
	}

	int getWidth() { return _width; }
	int getHeight() { return _height; }

	HANDLE get_conHandle() {
		return _conHandle;
	}

	bool isWork() {

		return _isWork;
	}

	void stop() {
		_isWork = false;
	}

	void drawBackground(COLORREF color = ConsoleColor::lighgreen()) {
		RECT rc;
		GetClientRect(_hwnd, &rc);
		drawRect(Point(), Point(_width, _height), color, true);
	}

	void drawPoint(Point p, COLORREF color) {
		SetPixel(_hdc, p.getX(), p.getY(), color);
	}

	void drawLine(Point p1, Point p2, COLORREF color) {
		selectPen(color);
		//SelectObject(_hdc, CreatePen(PS_SOLID, 1, color));
		//SetDCPenColor(_hdc, color);
		MoveToEx(_hdc, p1.getX(), p1.getY(), NULL);
		LineTo(_hdc, p2.getX(), p2.getY());
	}

	// Рисует круг или окружность по центру и радиусу
	// filled - нужно ли закрашивать внутренность
	void drawCircle(Point c, int radius, COLORREF color, bool filled = false) {
		selectPen(color);
		selectBrush(color, filled);
		//SetDCBrushColor(_hdc, color);
		Ellipse(_hdc, c.getX() - radius, c.getY() - radius, c.getX() + radius, c.getY() + radius);

	}

	// Рисует прямоугольник по двум точкам
	// p1 - нижний левый угол
	// p2 - верхний правый угол
	void drawRect(Point p1, Point p2, COLORREF color, bool filled = false) {
		selectPen(color);
		selectBrush(color, filled);
		Rectangle(_hdc, p1.getX(), p1.getY(), p2.getX(), p2.getY());
	}

	// Рисует треугольник по трем точкам
	void drawTriangle(Point p1, Point p2, Point p3, COLORREF color, bool filled = false) {
		selectPen(color);
		selectBrush(color, filled);
		POINT apt[3];
		apt[0].x = p1.getX(); apt[0].y = p1.getY();
		apt[1].x = p2.getX(); apt[1].y = p2.getY();
		apt[2].x = p3.getX(); apt[2].y = p3.getY();
		Polygon(_hdc, apt, 3);
	}
};

// Это самый базовый класс, от которого наследуются все объекты, которые появляются на поле
class ParkObject {
protected:
	int _cellX, _cellY, _cellSize;

public:
	ParkObject(int cellX, int cellY, int cellSize) {
		_cellX = cellX;
		_cellY = cellY;
		_cellSize = cellSize;
	}

	bool isInside(int cellX, int cellY) {
		return (_cellX == cellX) && (_cellY == cellY);
	}

	int getCellX() {
		return _cellX;
	}

	int getCellY() {
		return _cellY;
	}

	int getX() {
		return _cellX * _cellSize;
	}

	int getY() {
		return _cellY * _cellSize;
	}

};

class SnowMan : public ParkObject {

public:
	SnowMan(int cellX, int cellY, int cellSize) : ParkObject(cellX, cellY, cellSize) { }

	void draw(ConsoleDrawer& cd) {
		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();

		double d = _cellSize / 100.0;
		/*cd.drawCircle(Point(x + _cellSize / 2, y + _cellSize - 20 * d), 20 * d, ConsoleColor::white(), true);
		cd.drawCircle(Point(x + _cellSize / 2, y + _cellSize - 55 * d), 15 * d, ConsoleColor::white(), true);
		cd.drawCircle(Point(x + _cellSize / 2, y + _cellSize - 80 * d), 10 * d, ConsoleColor::white(), true);
		cd.drawRect(Point(x + _cellSize / 2 - 5 * d, y + 10 * d), Point(x + _cellSize / 2 + 5 * d, y + d), ConsoleColor::brown(), true);*/
		cd.drawRect(Point(x + 5 * d, y + 40 * d), Point(x + 65 * d, y + 10 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 65 * d, y + 40 * d), Point(x + 95 * d, y + 10 * d), ConsoleColor::orange(), true);
		cd.drawRect(Point(x + 5 * d, y + 70 * d), Point(x + 35 * d, y + 40 * d), ConsoleColor::orange(), true);
		cd.drawRect(Point(x + 35 * d, y + 70 * d), Point(x + 95 * d, y + 40 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 5 * d, y + 100 * d), Point(x + 65 * d, y + 70 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 65 * d, y + 100 * d), Point(x + 95 * d, y + 70 * d), ConsoleColor::orange(), true);
	}
};

class Obstacle : public ParkObject {
protected:
	int _cellX, _cellY, _cellSize;
public:
	Obstacle(int cellX, int cellY, int cellSize) : ParkObject(cellX, cellY, cellSize) {
		_cellX = cellX;
		_cellY = cellY;
		_cellSize = cellSize;
	}
};
class Tree1 : public Obstacle {

public:
	Tree1(int cellX, int cellY, int cellSize) : Obstacle(cellX, cellY, cellSize) {}

	void draw(ConsoleDrawer& cd) {
		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();

		double d = _cellSize / 100.0;
		/*int d = _cellSize / 10;
		cd.drawRect(Point(x + 4 * d, y + _cellSize), Point(x + 6 * d, y + _cellSize - 2 * d), ConsoleColor::brown(), true);
		cd.drawTriangle(Point(x + 2 * d, y + _cellSize - 2 * d), Point(x + _cellSize / 2, y + d), Point(x + _cellSize - 2 * d, y + _cellSize - 2 * d), ConsoleColor::green(), true);*/
		cd.drawRect(Point(x + 10 * d, y + _cellSize), Point(x + 90 * d, y + 30 * d), ConsoleColor::red(), true);
		cd.drawRect(Point(x + 40 * d, y + 90 * d), Point(x + 60 * d, y + 40 * d), ConsoleColor::white(), true);
		cd.drawRect(Point(x + 23 * d, y + 75 * d), Point(x + 78 * d, y + 55 * d), ConsoleColor::white(), true);
		cd.drawRect(Point(x + 23 * d, y + 33 * d), Point(x + 33 * d, y + 0 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 33 * d, y + 10 * d), Point(x + 67 * d, y + 0 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 67 * d, y + 33 * d), Point(x + 77 * d, y + 0 * d), ConsoleColor::brown(), true);
	}
};

class Tree2 : public Obstacle {

public:
	Tree2(int cellX, int cellY, int cellSize) : Obstacle(cellX, cellY, cellSize) {}

	void draw(ConsoleDrawer& cd) {
		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();

		double d = _cellSize / 100.0;
		/*cd.drawRect(Point(x + 4 * d, y + _cellSize), Point(x + 6 * d, y + _cellSize - 2 * d), ConsoleColor::brown(), true);
		cd.drawTriangle(Point(x + 2 * d, y + _cellSize - 2 * d), Point(x + _cellSize / 2, y + d), Point(x + _cellSize - 2 * d, y + _cellSize - 2 * d), ConsoleColor::blue(), true);*/
		cd.drawRect(Point(x + 10 * d, y + _cellSize), Point(x + 90 * d, y + 30 * d), ConsoleColor::blue(), true);
		cd.drawRect(Point(x + 40 * d, y + 90 * d), Point(x + 60 * d, y + 40 * d), ConsoleColor::yellow(), true);
		cd.drawRect(Point(x + 23 * d, y + 75 * d), Point(x + 78 * d, y + 55 * d), ConsoleColor::yellow(), true);
		cd.drawRect(Point(x + 23 * d, y + 33 * d), Point(x + 33 * d, y + 0 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 33 * d, y + 10 * d), Point(x + 67 * d, y + 0 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 67 * d, y + 33 * d), Point(x + 77 * d, y + 0 * d), ConsoleColor::brown(), true);
	}
};

class Tree3 : public Obstacle {

public:
	Tree3(int cellX, int cellY, int cellSize) : Obstacle(cellX, cellY, cellSize) {}

	void draw(ConsoleDrawer& cd) {
		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();

		double d = _cellSize / 100.0;
		/*cd.drawRect(Point(x + 4 * d, y + _cellSize), Point(x + 6 * d, y + _cellSize - 2 * d), ConsoleColor::brown(), true);
		cd.drawTriangle(Point(x + 2 * d, y + _cellSize - 2 * d), Point(x + _cellSize / 2, y + d), Point(x + _cellSize - 2 * d, y + _cellSize - 2 * d), ConsoleColor::black(), true);*/
		cd.drawRect(Point(x + 10 * d, y + _cellSize), Point(x + 90 * d, y + 30 * d), ConsoleColor::black(), true);
		cd.drawRect(Point(x + 40 * d, y + 90 * d), Point(x + 60 * d, y + 40 * d), ConsoleColor::green(), true);
		cd.drawRect(Point(x + 23 * d, y + 75 * d), Point(x + 78 * d, y + 55 * d), ConsoleColor::green(), true);
		cd.drawRect(Point(x + 23 * d, y + 33 * d), Point(x + 33 * d, y + 0 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 33 * d, y + 10 * d), Point(x + 67 * d, y + 0 * d), ConsoleColor::brown(), true);
		cd.drawRect(Point(x + 67 * d, y + 33 * d), Point(x + 77 * d, y + 0 * d), ConsoleColor::brown(), true);
	}
};

class Present : public Obstacle {

public:
	Present(int cellX, int cellY, int cellSize) :Obstacle(cellX, cellY, cellSize) {
	}

	void draw(ConsoleDrawer& cd) {
		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();

		double d = _cellSize / 100.0;
		/*cd.drawRect(Point(x + 20 * d, y + _cellSize), Point(x + 80 * d, y + 40 * d), ConsoleColor::red(), true);
		cd.drawRect(Point(x + 40 * d, y + _cellSize), Point(x + 60 * d, y + 40 * d), ConsoleColor::yellow(), true);
		cd.drawCircle(Point(x + 35 * d, y + 20 * d), 10 * d, ConsoleColor::yellow(), true);
		cd.drawCircle(Point(x + 65 * d, y + 20 * d), 10 * d, ConsoleColor::yellow(), true);
		cd.drawTriangle(Point(x + 50 * d, y + 40 * d), Point(x + 35 * d, y + 30 * d), Point(x + 45 * d, y + 20 * d), ConsoleColor::yellow(), true);
		cd.drawTriangle(Point(x + 50 * d, y + 40 * d), Point(x + 65 * d, y + 30 * d), Point(x + 55 * d, y + 20 * d), ConsoleColor::yellow(), true);*/
		cd.drawCircle(Point(x + 50 * d, y + 70 * d), 30 * d, ConsoleColor::darkgreen(), true);
		cd.drawRect(Point(x + 45 * d, y + 40 * d), Point(x + 55 * d, y + 5 * d), ConsoleColor::black(), true);
		cd.drawRect(Point(x + 45 * d, y + 15 * d), Point(x + 70 * d, y + 5 * d), ConsoleColor::black(), true);
		cd.drawRect(Point(x + 45 * d, y + 35 * d), Point(x + 70 * d, y + 25 * d), ConsoleColor::black(), true);
		cd.drawRect(Point(x + 65 * d, y + 35 * d), Point(x + 75 * d, y + 5 * d), ConsoleColor::black(), true);
	}
};

class Dinosour : public ParkObject {
protected:
	int _cellX, _cellY, _cellSize;
public:
	Dinosour(int cellX, int cellY, int cellSize) : ParkObject(cellX, cellY, cellSize) {
		_cellX = cellX;
		_cellY = cellY;
		_cellSize = cellSize;
	}
};
class HappyDinoNorm : public ParkObject {

public:
	HappyDinoNorm(int cellX, int cellY, int cellSize) : ParkObject(cellX, cellY, cellSize) {}

	void draw(ConsoleDrawer& cd) {

		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();
		COLORREF color = ConsoleColor::violent();

		double d = _cellSize / 125.0;
		/*cd.drawRect(Point(x + d, y + 63 * d), Point(x + 24 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 75 * d, y + 63 * d), Point(x + 98 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 93 * d), Point(x + 72 * d, y + 35 * d), color, true);//body
		cd.drawRect(Point(x + 27 * d, y + 124 * d), Point(x + 42 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 57 * d, y + 124 * d), Point(x + 72 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 32 * d), Point(x + 88 * d, y + 2 * d), color, true);//head
		cd.drawCircle(Point(x + 42 * d, y + 12 * d), 8 * d, ConsoleColor::gray(), true);
		cd.drawTriangle(Point(x + 27 * d, y), Point(x + 49 * d, y - 45 * d), Point(x + 72 * d, y), ConsoleColor::red(), true);
		cd.drawCircle(Point(x + 49 * d, y - 50 * d), 10 * d, ConsoleColor::white(), true);*/
		cd.drawCircle(Point(x + 58 * d, y - 8 * d), 33 * d, ConsoleColor::black(), true);
		cd.drawRect(Point(x + d, y + 63 * d), Point(x + 24 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 75 * d, y + 63 * d), Point(x + 98 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 93 * d), Point(x + 72 * d, y + 35 * d), color, true);//body
		cd.drawRect(Point(x + 27 * d, y + 124 * d), Point(x + 42 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 57 * d, y + 124 * d), Point(x + 72 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 32 * d), Point(x + 88 * d, y + 2 * d), color, true);//head
		cd.drawCircle(Point(x + 42 * d, y + 12 * d), 8 * d, ConsoleColor::gray(), true);
		cd.drawRect(Point(x + 27 * d, y + 10 * d), Point(x + 88 * d, y - 20 * d), ConsoleColor::black(), true);

	}

	void step1(int direction) {
		if (direction == 0)
			_cellX -= 1;
		if (direction == 1)
			_cellY -= 1;
		if (direction == 2)
			_cellX += 1;
		if (direction == 3)
			_cellY += 1;
	}
};

class HappyDinoAngry1 : public Dinosour {

public:
	HappyDinoAngry1(int cellX, int cellY, int cellSize) : Dinosour(cellX, cellY, cellSize) {}

	void draw(ConsoleDrawer& cd) {

		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();
		COLORREF color = ConsoleColor::red();

		double d = _cellSize / 125.0;
		cd.drawRect(Point(x + d, y + 63 * d), Point(x + 24 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 75 * d, y + 63 * d), Point(x + 98 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 93 * d), Point(x + 72 * d, y + 35 * d), color, true);//body
		cd.drawRect(Point(x + 27 * d, y + 124 * d), Point(x + 42 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 57 * d, y + 124 * d), Point(x + 72 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 32 * d), Point(x + 88 * d, y + 2 * d), color, true);//head
		cd.drawCircle(Point(x + 42 * d, y + 12 * d), 8 * d, ConsoleColor::gray(), true);
		cd.drawTriangle(Point(x + 27 * d, y), Point(x + 49 * d, y - 45 * d), Point(x + 72 * d, y), ConsoleColor::darkgreen(), true);

	}
};

class HappyDinoAngry2 : public Dinosour {

public:
	HappyDinoAngry2(int cellX, int cellY, int cellSize) : Dinosour(cellX, cellY, cellSize) {}

	void draw(ConsoleDrawer& cd) {

		//Переходим от ячеек к координатам пикселей
		int x = getX();
		int y = getY();
		COLORREF color = ConsoleColor::white();

		double d = _cellSize / 125.0;
		cd.drawRect(Point(x + d, y + 63 * d), Point(x + 24 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 75 * d, y + 63 * d), Point(x + 98 * d, y + 53 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 93 * d), Point(x + 72 * d, y + 35 * d), color, true);//body
		cd.drawRect(Point(x + 27 * d, y + 124 * d), Point(x + 42 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 57 * d, y + 124 * d), Point(x + 72 * d, y + 94 * d), color, true);
		cd.drawRect(Point(x + 27 * d, y + 32 * d), Point(x + 88 * d, y + 2 * d), color, true);//head
		cd.drawCircle(Point(x + 42 * d, y + 12 * d), 8 * d, ConsoleColor::gray(), true);
		cd.drawTriangle(Point(x + 27 * d, y), Point(x + 49 * d, y - 45 * d), Point(x + 72 * d, y), ConsoleColor::darkgreen(), true);

	}
};
int h = 0;
class DinoPark {
	int game = 0;
	int _cellsXCount;
	int _cellsYCount;
	int _cellSize;
	vector<SnowMan> _snowmen;
	vector<Tree1> _trees1;
	vector<Tree2> _trees2;
	vector<Tree3> _trees3;
	vector<Present> _present;
	vector<HappyDinoNorm> _hDinoN;
	vector<HappyDinoAngry1> _hDinoA1;
	vector<HappyDinoAngry2> _hDinoA2;

public:
	DinoPark(int width, int height, int cellSize) {
		_cellsXCount = width;
		_cellsYCount = height;
		_cellSize = cellSize;
	}

	// Этот метод проверяет, что находится в клетке:
	int find(int x, int y) {
		for (int i = 0; i < _snowmen.size(); i++) {
			if (_snowmen[i].isInside(x, y))
				return -1;
		}
		for (int i = 0; i < _trees1.size(); i++) {
			if (_trees1[i].isInside(x, y))
				return 11;
		}
		for (int i = 0; i < _trees2.size(); i++) {
			if (_trees2[i].isInside(x, y))
				return 12;
		}
		for (int i = 0; i < _trees3.size(); i++) {
			if (_trees3[i].isInside(x, y))
				return 13;
		}
		for (int i = 0; i < _present.size(); i++) {
			if (_present[i].isInside(x, y))
				return 3;
		}
		for (int i = 0; i < _hDinoN.size(); i++) {
			if (_hDinoN[i].isInside(x, y))
				return 2;
		}
		for (int i = 0; i < _hDinoA1.size(); i++) {
			if (_hDinoA1[i].isInside(x, y))
				return 22;
		}
		for (int i = 0; i < _hDinoA2.size(); i++) {
			if (_hDinoA2[i].isInside(x, y))
				return 23;
		}
		return 0;
	}

	// Метод для отрисовки всего поля
	void draw(ConsoleDrawer& cd) {

		// Рисуем сетку 1/1
		/*for (int i = 0; i <= _cellsXCount; i++) {
			int x = i * _cellSize;
			int y = _cellsYCount * _cellSize;
			cd.drawLine(Point(x, 0), Point(x, y), ConsoleColor::black());
		}
		for (int i = 0; i <= _cellsYCount; i++) {
			int x = _cellsXCount * _cellSize;
			int y = i * _cellSize;
			cd.drawLine(Point(0, y), Point(x, y), ConsoleColor::black());
		}*/
		// Рисуем сетку 2/2
		/*for (int i = 0; i <= _cellsXCount; i++) {
			int x = i * _cellSize * 2;
			int y = _cellsYCount * _cellSize;
			cd.drawLine(Point(x, 0), Point(x, y), ConsoleColor::black());
		}
		for (int i = 0; i <= _cellsYCount; i++) {
			int x = _cellsXCount * _cellSize;
			int y = i * _cellSize * 2;
			cd.drawLine(Point(0, y), Point(x, y), ConsoleColor::black());
		}*/

		// Рисуем снеговиков
		for (int i = 0; i < _snowmen.size(); i++) {
			_snowmen[i].draw(cd);
		}

		// Рисуем деревья
		for (int i = 0; i < _trees1.size(); i++) {
			_trees1[i].draw(cd);
		}
		for (int i = 0; i < _trees2.size(); i++) {
			_trees2[i].draw(cd);
		}
		for (int i = 0; i < _trees3.size(); i++) {
			_trees3[i].draw(cd);
		}
		for (int i = 0; i < _present.size(); i++) {
			_present[i].draw(cd);
		}
		// Рисуем динозавра
		for (int i = 0; i < _hDinoN.size(); i++) {
			_hDinoN[i].draw(cd);
		}
		for (int i = 0; i < _hDinoA1.size(); i++) {
			_hDinoA1[i].draw(cd);
		}
		for (int i = 0; i < _hDinoA2.size(); i++) {
			_hDinoA2[i].draw(cd);
		}
	}

	// Метод обработки вашего хода
	void step1(ConsoleDrawer& cd) {

		// Пока ходит только счастливый динозавр

		// Ловим нажатие на клавиатуру
		KEY_EVENT_RECORD key;
		INPUT_RECORD irec[100];
		DWORD cc;

		ReadConsoleInput(cd.get_conHandle(), irec, 100, &cc);
		for (DWORD j = 0; j < cc; ++j) {
			if (irec[j].EventType == KEY_EVENT && irec[j].Event.KeyEvent.bKeyDown) {

				for (int i = 0; i < _hDinoN.size(); i++) {
					// Смотрим по сторонам
					// -1 - снеговик
					//  0 - пусто
					//  1 - дерево
					//  2 - динозавр
					// 22 - злой динозавр
					vector<int> res = {
						lookLeft(_hDinoN[i].getCellX(), _hDinoN[i].getCellY()),
						lookUp(_hDinoN[i].getCellX(), _hDinoN[i].getCellY()),
						lookRight(_hDinoN[i].getCellX(), _hDinoN[i].getCellY()),
						lookDown(_hDinoN[i].getCellX(), _hDinoN[i].getCellY())
					};
					// Проверяем, какая именно кнопка была нажата
					switch (irec[j].Event.KeyEvent.wVirtualKeyCode) {
					case VK_LEFT:
						// если путь свободен, идем
						if (res[0] != -1) {
							_hDinoN[i].step1(0);
							// если мы на что-то наступили (пока что только дерево), обновляем картину
							if (res[0] == 11) {
								refresh(cd);
								game += 10;
							}
							if (res[0] == 13) {
								refresh(cd);
								game += 50;
							}
							if (res[0] == 12) {
								refresh(cd);
								game += 20;
							}
							if (res[0] == 3) {
								refresh(cd);
								game += 30;
							}
							if (res[0] == 22) {
								cout << "Your point is:" << game << endl;
								system("pause");
								cd.stop();
								break;
							}
							if (res[0] == 23) {
								refresh(cd);
								game -= 300;
							}
							else game -= 1;
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA1[j] = HappyDinoAngry1(_x, _y, _cellSize);
									break;
								}
							}
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA2[j] = HappyDinoAngry2(_x, _y, _cellSize);
									break;
								}
							}
						}
						break;
					case VK_UP:
						if (res[1] != -1) {
							_hDinoN[i].step1(1);
							if (res[1] == 11) {
								refresh(cd);
								game += 10;
							}
							if (res[1] == 13) {
								refresh(cd);
								game += 50;
							}
							if (res[1] == 12) {
								refresh(cd);
								game += 20;
							}
							if (res[1] == 3) {
								refresh(cd);
								game += 30;
							}
							if (res[1] == 22) {
								cout << "Your point is:" << game << endl;
								system("pause");
								cd.stop();
								break;
							}
							if (res[1] == 23) {
								refresh(cd);
								game -= 300;
							}
							else game -= 1;
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA1[j] = HappyDinoAngry1(_x, _y, _cellSize);
									break;
								}
							}
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA2[j] = HappyDinoAngry2(_x, _y, _cellSize);
									break;
								}
							}
						}
						break;
					case VK_RIGHT:
						if (res[2] != -1) {
							_hDinoN[i].step1(2);
							if (res[2] == 11) {
								refresh(cd);
								game += 10;
							}
							if (res[2] == 13) {
								refresh(cd);
								game += 50;
							}
							if (res[2] == 12) {
								refresh(cd);
								game += 20;
							}
							if (res[2] == 3) {
								refresh(cd);
								game += 30;
							}
							if (res[2] == 22) {
								cout << "Your point is:" << game << endl;
								system("pause");
								cd.stop();
								break;
							}
							if (res[2] == 23) {
								refresh(cd);
								game -= 300;
							}
							else game -= 1;
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA1[j] = HappyDinoAngry1(_x, _y, _cellSize);
									break;
								}
							}
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA2[j] = HappyDinoAngry2(_x, _y, _cellSize);
									break;
								}
							}

						}
						break;
					case VK_DOWN:
						if (res[3] != -1) {
							_hDinoN[i].step1(3);
							if (res[3] == 11) {
								refresh(cd);
								game += 10;
							}
							if (res[3] == 13) {
								refresh(cd);
								game += 50;
							}
							if (res[3] == 12) {
								refresh(cd);
								game += 20;
							}
							if (res[3] == 3) {
								refresh(cd);
								game += 30;
							}
							if (res[3] == 22) {
								cout << "Your point is:" << game << endl;
								system("pause");
								cd.stop();
								break;
							}
							if (res[3] == 23) {
								refresh(cd);
								game -= 300;
							}
							else game -= 1;
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA1[j] = HappyDinoAngry1(_x, _y, _cellSize);
									break;
								}
							}
							for (int k = 0; k < 100; k++) {
								int _x = rand() % _cellsXCount;
								int _y = rand() % _cellsYCount;
								if (find(_x, _y) == 0 || find(_x, _y) == 2) {
									_hDinoA2[j] = HappyDinoAngry2(_x, _y, _cellSize);
									break;
								}
							}
						}
						break;
					case VK_ESCAPE:
						//if (h == 0) {
						cout << "Your point is:" << game << endl;
						system("pause");
						//}
						//else {
						cd.stop();
						break;
						//}
					}

				}

			}
		}
	}

	void addSnowMan(int x, int y) {
		_snowmen.push_back(SnowMan(x, y, _cellSize));
	}
	void addTree1(int x, int y) {
		_trees1.push_back(Tree1(x, y, _cellSize));
	}
	void addTree2(int x, int y) {
		_trees2.push_back(Tree2(x, y, _cellSize));
	}
	void addTree3(int x, int y) {
		_trees3.push_back(Tree3(x, y, _cellSize));
	}
	void addPresent(int x, int y) {
		_present.push_back(Present(x, y, _cellSize));
	}
	void addHappyDinoN(int x, int y) {
		_hDinoN.push_back(HappyDinoNorm(x, y, _cellSize));
	}
	void addHappyDinoA1(int x, int y) {
		_hDinoA1.push_back(HappyDinoAngry1(x, y, _cellSize));
	}
	void addHappyDinoA2(int x, int y) {
		_hDinoA2.push_back(HappyDinoAngry2(x, y, _cellSize));
	}

	// Взгляд на клетку вверх
	int lookUp(int cellX, int cellY) {
		if (cellY == 0)
			return -1;
		return find(cellX, cellY - 1);
	}

	// Взгляд на клетку вниз
	int lookDown(int cellX, int cellY) {
		if (cellY == _cellsYCount - 1)
			return -1;
		return find(cellX, cellY + 1);
	}

	// Взгляд на клетку вправо
	int lookRight(int cellX, int cellY) {
		if (cellX == _cellsXCount - 1)
			return -1;
		return find(cellX + 1, cellY);
	}

	// Взгляд на клетку влево
	int lookLeft(int cellX, int cellY) {
		if (cellX == 0)
			return -1;
		return find(cellX - 1, cellY);
	}

	// Обновляем картину
	void refresh(ConsoleDrawer& cd) {
		for (int i = 0; i < _hDinoN.size(); i++) {
			// Смотрим, где стоит динозавр
			int x = _hDinoN[i].getCellX();
			int y = _hDinoN[i].getCellY();
			// Находим то дерево, на котором он стоит
			for (int j = 0; j < _trees1.size(); j++) {
				if (_trees1[j].isInside(x, y)) {
					// Переносим дерево в другое место
					// Цикл нужен, чтобы не попасть в место, которое уже занято
					for (int k = 0; k < 100; k++) {
						int _x = rand() % _cellsXCount;
						int _y = rand() % _cellsYCount;
						if (find(_x, _y) == 0) {
							_trees1[j] = Tree1(_x, _y, _cellSize);
							break;
						}
					}
				}
			}
			for (int j = 0; j < _trees2.size(); j++) {
				if (_trees2[j].isInside(x, y)) {
					// Переносим дерево в другое место
					// Цикл нужен, чтобы не попасть в место, которое уже занято
					for (int k = 0; k < 100; k++) {
						int _x = rand() % _cellsXCount;
						int _y = rand() % _cellsYCount;
						if (find(_x, _y) == 0) {
							_trees2[j] = Tree2(_x, _y, _cellSize);
							break;
						}
					}
				}
			}
			for (int j = 0; j < _trees3.size(); j++) {
				if (_trees3[j].isInside(x, y)) {
					// Переносим дерево в другое место
					// Цикл нужен, чтобы не попасть в место, которое уже занято
					for (int k = 0; k < 100; k++) {
						int _x = rand() % _cellsXCount;
						int _y = rand() % _cellsYCount;
						if (find(_x, _y) == 0) {
							_trees3[j] = Tree3(_x, _y, _cellSize);
							break;
						}
					}
				}
			}
			for (int j = 0; j < _present.size(); j++) {
				if (_present[j].isInside(x, y)) {
					// Переносим дерево в другое место
					// Цикл нужен, чтобы не попасть в место, которое уже занято
					for (int k = 0; k < 100; k++) {
						int _x = rand() % _cellsXCount;
						int _y = rand() % _cellsYCount;
						if (find(_x, _y) == 0) {
							_present[j] = Present(_x, _y, _cellSize);
							break;
						}
					}
				}
			}
		}
	}

};

int main() {
	ConsoleDrawer cd;
	srand(time(0));

	const int CellSize = 40;
	int width = cd.getWidth() / CellSize;
	int height = cd.getHeight() / CellSize;
	DinoPark dinoPark(width, height, CellSize);

	//Снеговики по вертикали
	/*dinoPark.addSnowMan(8, 1);
	dinoPark.addSnowMan(8, 2);
	dinoPark.addSnowMan(8, 3);
	dinoPark.addSnowMan(8, 4);
	dinoPark.addSnowMan(8, 5);
	dinoPark.addSnowMan(8, 6);
	dinoPark.addSnowMan(8, 7);
	dinoPark.addSnowMan(8, 8);*/
	//Деревья по горизонтали
	/*dinoPark.addTree(0, 0);
	dinoPark.addTree(1, 0);
	dinoPark.addTree(2, 0);
	dinoPark.addTree(3, 0);
	dinoPark.addTree(4, 0);
	dinoPark.addTree(5, 0);*/

	dinoPark.addHappyDinoN(1, 0);
	dinoPark.addHappyDinoA1(11, 5);
	dinoPark.addHappyDinoA2(12, 5);

	dinoPark.addSnowMan(0, 1);
	dinoPark.addSnowMan(1, 1);
	dinoPark.addSnowMan(2, 2);
	dinoPark.addSnowMan(3, 3);
	dinoPark.addSnowMan(4, 4);
	dinoPark.addSnowMan(5, 5);
	dinoPark.addSnowMan(6, 6);
	dinoPark.addSnowMan(7, 7);
	dinoPark.addSnowMan(8, 8);
	dinoPark.addSnowMan(9, 9);
	dinoPark.addSnowMan(10, 10);

	dinoPark.addSnowMan(18, 0);
	dinoPark.addSnowMan(19, 1);
	dinoPark.addSnowMan(20, 2);

	dinoPark.addTree1(1, 2);
	dinoPark.addTree2(2, 3);
	dinoPark.addTree1(3, 4);
	dinoPark.addTree2(4, 5);
	dinoPark.addTree1(5, 6);
	dinoPark.addTree2(6, 7);
	dinoPark.addTree1(7, 8);
	dinoPark.addTree2(8, 9);

	dinoPark.addTree3(19, 0);
	dinoPark.addTree3(20, 0);
	dinoPark.addTree3(20, 1);
	dinoPark.addTree3(17, 0);

	dinoPark.addPresent(15, 1);
	dinoPark.addPresent(15, 2);
	dinoPark.addPresent(15, 3);
	cd.clearScreen();

	while (cd.isWork()) {
		cd.drawBackground();
		dinoPark.draw(cd);
		dinoPark.step1(cd);
	}
	//"Зелёные елочки +10"
	//"Красные елочки +20" 
	//"Черные елочки +50"
	//"Подарки +30"
	return 0;
}