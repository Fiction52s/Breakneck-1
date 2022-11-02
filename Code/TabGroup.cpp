#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

TabGroup::TabGroup(const std::string &n, int posx, int posy, std::vector<std::string> &tabStrings, int memberWidth, int height, sf::Font &f, Panel *p)
	:PanelMember( p), pos(posx, posy), characterHeight(height - 4), memberSize(memberWidth, height), name(n)
{
	numTabs = tabStrings.size();
	tabNames.resize(numTabs);

	totalSize.x = memberSize.x * numTabs;
	totalSize.y = memberSize.y;

	quads = new sf::Vertex[4 * numTabs];

	for (int i = 0; i < numTabs; ++i)
	{
		tabNames[i].setString(tabStrings[i]);
		tabNames[i].setFont(f);
		tabNames[i].setFillColor(Color::White);
		tabNames[i].setCharacterSize(characterHeight);

		auto bounds = tabNames[i].getLocalBounds();
		tabNames[i].setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
	}

	selectedColor = Color(0, 122, 204);
	unselectedColor = Color(51, 51, 55);

	SelectTab(0);

	SetPos(pos);
}

TabGroup::~TabGroup()
{
	delete[] quads;
}

void TabGroup::Deactivate()
{
	//clickedDown = false;
}

void TabGroup::SetPos(sf::Vector2i &p_pos)
{
	pos = p_pos;

	Vector2f tabCenter;
	for (int i = 0; i < numTabs; ++i)
	{
		tabCenter = Vector2f(pos.x + memberSize.x * i + memberSize.x / 2, pos.y + memberSize.y / 2);
		tabNames[i].setPosition(tabCenter);

		SetRectCenter(quads + i * 4, memberSize.x, memberSize.y, tabCenter);
	}
}

void TabGroup::SelectTab(int index)
{
	currTabIndex = index;

	for (int i = 0; i < numTabs; ++i)
	{
		if (i == currTabIndex)
		{
			SetRectColor(quads + i * 4, selectedColor);
		}
		else
		{
			SetRectColor(quads + i * 4, unselectedColor);
		}
		
	}
}

bool TabGroup::MouseUpdate()
{
	if (hidden)
		return false;

	Vector2i mousePos = panel->GetMousePos();

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
	}

	return false;
}

void TabGroup::Draw(RenderTarget *target)
{
	if (hidden)
		return;

	target->draw(quads, 4 * numTabs, sf::Quads);
	for (int i = 0; i < numTabs; ++i)
	{
		target->draw(tabNames[i]);
	}
}