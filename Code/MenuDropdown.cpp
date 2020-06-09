#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

MenuDropdown::MenuDropdown(const std::string &n, sf::Vector2i &p_pos,
	sf::Vector2i &p_size, int p_optionWidth, sf::Font &f,
	const std::vector<std::string> &p_options, Panel *p)
	:PanelMember(p), pos(p_pos), clickedDown(false), characterHeight(size.y - 4), size(p_size), name(n),
	myFont(f), expanded(false), optionWidth(p_optionWidth)
{
	selectedIndex = -1;
	SetOptions(p_options);

	idleColor = Color::Black;
	mouseOverColor = Color(100, 100, 100);
	expandedColor = Color(150, 150, 150);
}

MenuDropdown::~MenuDropdown()
{
	delete[] dropdownRects;
}

void MenuDropdown::Deactivate()
{
	SetExpanded(false, false);
	clickedDown = false;
}

void MenuDropdown::SetExpanded(bool exp, bool onMain)
{
	expanded = exp;
	if (expanded)
	{
		SetRectColor(mainRect, expandedColor);
	}
	else
	{
		if (onMain)
		{
			SetRectColor(mainRect, mouseOverColor);
		}
		else
		{
			SetRectColor(mainRect, idleColor);
		}

	}
}

void MenuDropdown::SetOptions(const std::vector<std::string> &p_options)
{
	options = p_options;

	numOptions = options.size();
	optionText.resize(numOptions);

	SetRectTopLeft(mainRect, size.x, size.y, Vector2f(pos));
	SetRectColor(mainRect, idleColor);

	menuText.setString(name);
	menuText.setFont(myFont);
	menuText.setFillColor(Color::White);
	menuText.setCharacterSize(characterHeight);
	auto lb = menuText.getLocalBounds();
	menuText.setOrigin(lb.left, lb.top);
	menuText.setPosition(Vector2f(pos) + Vector2f(4, 4));

	dropdownRects = new Vertex[4 * numOptions];

	for (int i = 0; i < numOptions; ++i)
	{
		Vector2f dropPos(pos.x, pos.y + size.y * (i + 1));
		SetRectTopLeft(dropdownRects + i * 4, optionWidth, size.y, dropPos);
		SetRectColor(dropdownRects + i * 4, Color(Color::Blue));

		Text &t = optionText[i];
		t.setString(options[i]);
		t.setFont(myFont);
		t.setFillColor(Color::White);
		t.setCharacterSize(characterHeight);
		auto tlb = t.getLocalBounds();
		t.setOrigin(tlb.left, tlb.top);
		t.setPosition(dropPos + Vector2f(4, 4));
	}
}

void MenuDropdown::Draw(sf::RenderTarget *target)
{
	target->draw(mainRect, 4, sf::Quads);
	target->draw(menuText);

	if (expanded)
	{
		target->draw(dropdownRects, numOptions * 4, sf::Quads);
		for (int i = 0; i < numOptions; ++i)
		{
			target->draw(optionText[i]);
		}
	}
}

void MenuDropdown::SetSelectedIndex(int ind)
{
	if (ind != selectedIndex)
	{
		selectedIndex = ind;
	}
}

bool MenuDropdown::IsMouseOnOption(int ind, Vector2f &point)
{
	return QuadContainsPoint(dropdownRects + ind * 4, point);
}

bool MenuDropdown::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	Vector2f point(mousePos);

	bool onMainQuad = QuadContainsPoint(mainRect, point);
	if (onMainQuad)
	{
		panel->SetFocusedMember(this);
	}
	//UpdateToolTip(onMainQuad);

	if (expanded)
	{
		int highlightedIndex = -1;
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point))
			{
				highlightedIndex = i;
				break;
			}
		}

		for (int i = 0; i < numOptions; ++i)
		{
			if (i == highlightedIndex)
			{
				SetRectColor(dropdownRects + i * 4, Color(Color::Red));
			}
			else
			{
				SetRectColor(dropdownRects + i * 4, Color(Color::Blue));
			}
		}
	}



	if (!expanded)
	{
		if (onMainQuad)
		{
			SetRectColor(mainRect, mouseOverColor);
		}
		else
		{
			SetRectColor(mainRect, idleColor);
		}
	}


	if (MOUSE.IsMouseLeftReleased() && expanded)
	{
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point))
			{
				SetSelectedIndex(i);
				SetExpanded(false, onMainQuad);
				clickedDown = false;
				panel->SendEvent(this, "selected");
				return true;
			}
		}

		if (!onMainQuad)
		{
			SetExpanded(false, onMainQuad);
			clickedDown = false;
		}

		return false;
	}

	if (MOUSE.IsMouseLeftClicked())
	{
		if (expanded)
		{
			for (int i = 0; i < numOptions; ++i)
			{
				if (IsMouseOnOption(i, point))
				{
					SetSelectedIndex(i);
					SetExpanded(false, onMainQuad);
					clickedDown = true;
					panel->SendEvent(this, "selected");
					return true;
				}
			}

			clickedDown = true;
			SetExpanded(false, onMainQuad);
			return false;
		}
		else
		{
			if (onMainQuad)
			{
				clickedDown = true;
				SetExpanded(true, onMainQuad);
				return true;
			}
		}
	}
	else
	{
		clickedDown = false;
	}
	return false;
}