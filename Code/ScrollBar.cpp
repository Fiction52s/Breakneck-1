#include "GUI.h"
#include <assert.h>
#include <iostream>
//#include "Session.h"

using namespace sf;
using namespace std;

ScrollBar::ScrollBar(const std::string &n, sf::Vector2i &p_pos, sf::Vector2i &p_size, int p_numRows, int p_numDisplayedRows, Panel *p)
	:PanelMember(p), pos(p_pos), size(p_size), name(n)
{
	selectorColor = Color(0, 122, 204);
	bgColor = Color(51, 51, 55);

	SetPos(pos);

	SetRectColor(quads + 0, bgColor);
	SetRectColor(quads + 4, selectorColor);

	SetRows(p_numRows, p_numDisplayedRows);
	clickedDown = false;
}

ScrollBar::~ScrollBar()
{
}

void ScrollBar::Deactivate()
{
	//clickedDown = false;
}

void ScrollBar::SetRows(int p_numRows, int p_numDisplayedRows)
{
	numRows = p_numRows;
	numDisplayedRows = p_numDisplayedRows;

	if (numRows <= p_numDisplayedRows)
	{
		HideMember();
	}
	else
	{
		ShowMember();
	}

	maxIndex = numRows - numDisplayedRows;

	int spacing = size.y / (float)numRows;
	cursorHeight = spacing * (numRows - maxIndex);

	SetIndex(0);
}

void ScrollBar::SetIndex(int ind)
{
	int spacing = size.y / (float)numRows;

	if (ind < 0)
	{
		ind = 0;
	}
	else if (ind > maxIndex)
	{
		ind = maxIndex;
	}

	currIndex = ind;
	cursorPos = pos + Vector2i(0, spacing * currIndex);
	SetRectTopLeft(quads + 4, size.x, cursorHeight, Vector2f(cursorPos));
}

void ScrollBar::SetPos(sf::Vector2i &p_pos)
{
	pos = p_pos;

	SetRectTopLeft(quads, size.x, size.y, Vector2f(pos));
	SetIndex(currIndex);
}

bool ScrollBar::MouseUpdate()
{
	if (hidden)
		return false;

	int spacing = size.y / (float)numRows;

	Vector2i mousePos = panel->GetMousePos();

	//sf::Rect<int> r(pos.x, pos.y, size.x, size.y);
	sf::Rect<int> r(cursorPos.x, cursorPos.y, size.x, cursorHeight);

	Vector2i adjustedMousePos = mousePos - pos;

	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if (MOUSE.IsMouseLeftClicked())
	{
		if (containsMouse)
		{
			clickedDown = true;
			clickedPos = mousePos;
			clickedIndex = currIndex;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if (MOUSE.IsMouseDownLeft() && clickedDown)
		{
			int delta = (mousePos.y - clickedPos.y) / spacing;

			SetIndex(clickedIndex + delta);
			panel->SendEvent(this, "indexchange");
		}
		else
		{
			clickedDown = false;
		}
	}
	return false;
}

void ScrollBar::Draw(RenderTarget *target)
{
	if (hidden)
		return;

	target->draw(quads, 4 * 2, sf::Quads);
	/*target->draw(quads, 4 * numTabs, sf::Quads);
	for (int i = 0; i < numTabs; ++i)
	{
		target->draw(tabNames[i]);
	}*/
}