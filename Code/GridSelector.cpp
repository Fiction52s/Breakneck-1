#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

GridSelector::GridSelector(Vector2i p_pos, int xSizep, int ySizep, int iconX, int iconY, bool p_displaySelected,
	bool p_displayMouseOver, Panel *p)
	:PanelMember(p), xSize(xSizep), ySize(ySizep), tileSizeX(iconX), tileSizeY(iconY), active(true)
{
	displaySelected = p_displaySelected;
	displayMouseOver = p_displayMouseOver;
	icons = new Sprite *[xSize];
	names = new string *[xSize];
	for (int i = 0; i < xSize; ++i)
	{
		icons[i] = new Sprite[ySize];
		names[i] = new string[ySize];
		for (int j = 0; j < ySize; ++j)
		{
			icons[i][j].setTextureRect(sf::IntRect(0, 0, tileSizeX, tileSizeY));
			//icons[i][j].setPosition( i * tileSizeX, j * tileSizeY );
			names[i][j] = "not set";
		}
	}

	pos.x = p_pos.x;
	pos.y = p_pos.y;
	focusX = -1;
	focusY = -1;
	//selectedX = -1;
	//selectedY = -1;
	selectedX = 0;
	selectedY = 0;
	mouseOverX = -1;
	mouseOverY = -1;
}

GridSelector::~GridSelector()
{
	for (int i = 0; i < xSize; ++i)
	{
		delete[] icons[i];
		delete[] names[i];
	}
	delete[] icons;
	delete[] names;
}

void GridSelector::Deactivate()
{
	focusX = -1;
	focusY = -1;
	mouseOverX = -1;
	mouseOverY = -1;
	selectedX = -1;
	selectedY = -1;
}

void GridSelector::Set(int xi, int yi, Sprite s, const std::string &name)
{
	icons[xi][yi] = s;
	icons[xi][yi].setPosition(xi * tileSizeX, yi * tileSizeY);
	names[xi][yi] = name;
}

void GridSelector::Draw(sf::RenderTarget *target)
{
	if (hidden)
		return;

	if (active)
	{
		sf::RectangleShape rs;
		rs.setSize(Vector2f(xSize * tileSizeX, ySize * tileSizeY));
		rs.setFillColor(Color::Yellow);
		Vector2i truePos(pos.x, pos.y);
		rs.setPosition(truePos.x, truePos.y);

		target->draw(rs);

		for (int x = 0; x < xSize; ++x)
		{
			for (int y = 0; y < ySize; ++y)
			{
				Sprite &s = icons[x][y];
				Vector2f realPos = s.getPosition();
				s.setPosition(Vector2f(realPos.x + truePos.x, realPos.y + truePos.y));

				target->draw(s);
				//s.setColor( Color::White );
				s.setPosition(realPos);
			}
		}

		if (displaySelected)//selectedX >= 0 && selectedY >= 0 )
		{
			Sprite &s = icons[selectedX][selectedY];
			Vector2f rectPos = s.getPosition() + Vector2f(truePos.x, truePos.y);
			//s.setPosition( Vector2f( realPos.x + truePos.x, realPos.y + truePos.y ) );
			sf::RectangleShape re;
			re.setFillColor(Color::Transparent);
			re.setOutlineColor(Color::Green);
			re.setOutlineThickness(3);
			re.setPosition(rectPos.x, rectPos.y);
			re.setSize(Vector2f(tileSizeX, tileSizeY));
			target->draw(re);
		}

	}
}

//returns true if a selection has been made
bool GridSelector::MouseUpdate()
{
	if (hidden)
		return false;

	Vector2i mousePos = panel->GetMousePos();
	//cout << "update: " << posx << ", " << posy << endl;
	if (!active)
	{
		return false;
		//assert( false && "trying to update inactive grid selector" );
	}
	if (MOUSE.IsMouseLeftClicked())
	{
		sf::Rect<int> r(pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY);
		if (r.contains(mousePos))
		{
			focusX = (mousePos.x - pos.x) / tileSizeX;
			focusY = (mousePos.y - pos.y) / tileSizeY;
			cout << "contains index: " << focusX << ", " << focusY << endl;
		}
		else
		{
			focusX = -1;
			focusY = -1;
		}
	}
	else
	{
		sf::Rect<int> r(pos.x, pos.y, xSize * tileSizeX, ySize * tileSizeY);
		if (r.contains(mousePos))
		{
			int tempX = (mousePos.x - pos.x) / tileSizeX;
			int tempY = (mousePos.y - pos.y) / tileSizeY;
			if (tempX == focusX && tempY == focusY)
			{
				selectedX = tempX;
				selectedY = tempY;
				cout << "tempX: " << tempX << ", tempY: " << tempY << endl;
				panel->SendEvent(this, names[tempX][tempY]);//->GridSelectorCallback( this, names[tempX][tempY] );
				return true;
				//		cout << "success!" << endl;
			}
			else
			{
				focusX = -1;
				focusY = -1;
			}
			//cout << "contains index: " << posx / tileSizeX << ", " << posy / tileSizeY << endl;		

		}
		else
		{
			//	cout << "doesn't contain!" << endl;
			//	cout << "pos: " << posx << ", " << posy << endl;
			focusX = -1;
			focusY = -1;
		}
	}

	return false;
}