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
	SetIndex(0);
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
}

void ScrollBar::SetIndex(int ind)
{
	int spacing = size.y / (float)numRows;

	currIndex = ind;
	SetRectTopLeft(quads + 4, size.x, cursorHeight, Vector2f(pos + Vector2i(0, spacing * currIndex)));
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

	/*Vector2i mousePos = panel->GetMousePos();

	sf::Rect<int> r(pos.x, pos.y, totalSize.x, totalSize.y);

	Vector2i adjustedMousePos = mousePos - pos;

	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if (MOUSE.IsMouseLeftClicked())
	{
		if (containsMouse)
		{
			int mouseIndex = adjustedMousePos.x / memberSize.x;

			mouseIndex = max(0, mouseIndex);
			mouseIndex = min(numTabs, mouseIndex);

			SelectTab(mouseIndex);

			panel->SendEvent(this, "tabselected");
		}
	}*/

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