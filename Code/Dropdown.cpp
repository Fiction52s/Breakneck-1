#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

Dropdown::Dropdown(const std::string &n, sf::Vector2i &p_pos,
	sf::Vector2i &p_size, sf::Font &f,
	const std::vector<std::string> &p_options, int p_defaultIndex, Panel *p)
	:PanelMember(p), pos(p_pos), clickedDown(false), characterHeight(size.y - 4), size(p_size), name(n),
	myFont(f), defaultIndex(p_defaultIndex), expanded(false)
{
	selectedIndex = -1;
	SetOptions(p_options);
}

Dropdown::~Dropdown()
{
	delete[] dropdownRects;
}

bool Dropdown::ContainsMouse()
{
	Vector2i mousePos = panel->GetMousePos();
	Vector2f point(mousePos);

	bool onMainQuad = QuadContainsPoint(mainRect, point);
	if (!expanded)
	{
		return onMainQuad;
	}

	if (!onMainQuad)
	{
		bool onOption = false;
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point))
			{
				onOption = true;
				return true;
			}
		}
		return false;
	}
	else
	{
		return true;
	}
}

void Dropdown::Deactivate()
{
	expanded = false;
	clickedDown = false;
}

void Dropdown::SetOptions(const std::vector<std::string> &p_options)
{
	options = p_options;

	numOptions = options.size();
	optionText.resize(numOptions);

	SetRectTopLeft(mainRect, size.x, size.y, Vector2f(pos));
	SetRectColor(mainRect, Color(Color::Black));

	SetSelectedIndex(defaultIndex);
	selectedText.setFont(myFont);
	selectedText.setFillColor(Color::White);
	selectedText.setCharacterSize(characterHeight);
	auto lb = selectedText.getLocalBounds();
	selectedText.setOrigin(lb.left, lb.top);
	selectedText.setPosition(Vector2f(pos) + Vector2f(4, 4));

	dropdownRects = new Vertex[4 * numOptions];

	for (int i = 0; i < numOptions; ++i)
	{
		Vector2f dropPos(pos.x, pos.y + size.y * (i + 1));
		SetRectTopLeft(dropdownRects + i * 4, size.x, size.y, dropPos);
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

int Dropdown::GetIndex(const std::string &s)
{
	for (int i = 0; i < options.size(); ++i)
	{
		if (s == options[i])
		{
			return i;
		}
	}

	return -1;
}

void Dropdown::Draw(sf::RenderTarget *target)
{
	if (hidden)
		return;

	target->draw(mainRect, 4, sf::Quads);
	target->draw(selectedText);

	if (expanded)
	{
		target->draw(dropdownRects, numOptions * 4, sf::Quads);
		for (int i = 0; i < numOptions; ++i)
		{
			target->draw(optionText[i]);
		}
	}

	/*sf::RectangleShape rs;
	rs.setSize(size);
	rs.setPosition(pos.x, pos.y);
	if (clickedDown)
	rs.setFillColor(Color::Green);
	else
	rs.setFillColor(Color::Blue);

	target->draw(rs);

	target->draw(text);*/
}

void Dropdown::SetSelectedIndex(int ind)
{
	if (ind != selectedIndex)
	{
		selectedIndex = ind;
		selectedText.setString(options[selectedIndex]);
	}
}

bool Dropdown::SetSelectedText(
	const std::string &text)
{
	int numOptions = options.size();
	for (int i = 0; i < numOptions; ++i)
	{
		if (options[i] == text)
		{
			if (selectedIndex != i)
			{
				selectedIndex = i;
				selectedText.setString(options[i]);
			}

			return true;
		}
	}

	return false;
}

const std::string &Dropdown::GetSelectedText()
{
	return options[selectedIndex];
}

bool Dropdown::IsMouseOnOption(int ind, Vector2f &point)
{
	return QuadContainsPoint(dropdownRects + ind * 4, point);
}

bool Dropdown::MouseUpdate()
{
	if (hidden)
		return false;

	Vector2i mousePos = panel->GetMousePos();
	Vector2f point(mousePos);

	bool onMainQuad = QuadContainsPoint(mainRect, point);


	if (!expanded && onMainQuad)
	{
		panel->SetFocusedMember(this);
	}
	else if (expanded || !onMainQuad)//|| (!expanded && !onMainQuad) )
	{
		if (panel->focusedMember == this)
		{
			panel->focusedMember = NULL;
			panel->HideToolTip();
		}
	}

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

		if (highlightedIndex < 0)
		{
			highlightedIndex = selectedIndex;
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




	if (MOUSE.IsMouseLeftReleased() && expanded)
	{
		for (int i = 0; i < numOptions; ++i)
		{
			if (IsMouseOnOption(i, point))
			{
				SetSelectedIndex(i);
				expanded = false;
				clickedDown = false;
				panel->SendEvent(this, "selected");
				return true;
			}
		}

		if (!onMainQuad)
		{
			expanded = false;
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
					expanded = false;
					clickedDown = true;
					panel->SendEvent(this, "selected");
					return true;
				}
			}

			clickedDown = true;
			expanded = false;
			return false;
		}
		else
		{
			if (onMainQuad)
			{
				clickedDown = true;
				expanded = true;
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