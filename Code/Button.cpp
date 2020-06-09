#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

Button::Button(const string &n, int posx, int posy, int width, int height, sf::Font &f, const std::string & t, Panel *p)
	:PanelMember(p), pos(posx, posy), clickedDown(false), characterHeight(size.y - 4), size(width, height), name(n)
{
	text.setString(t);
	text.setFont(f);
	text.setFillColor(Color::White);
	text.setCharacterSize(characterHeight);

	auto bounds = text.getLocalBounds();
	text.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
	text.setPosition(pos.x + size.x / 2, pos.y + size.y / 2);
}

void Button::Deactivate()
{
	clickedDown = false;
}

bool Button::MouseUpdate()
{
	Vector2i mousePos = panel->GetMousePos();
	sf::Rect<int> r(pos.x, pos.y, size.x, size.y);

	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if (MOUSE.IsMouseLeftClicked())
	{
		if (containsMouse)
		{
			clickedDown = true;
		}
		else
		{
			clickedDown = false;
		}
	}
	else
	{
		if (containsMouse && clickedDown)
		{
			clickedDown = false;
			panel->SendEvent(this, "pressed");
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}

void Button::Draw(RenderTarget *target)
{
	sf::RectangleShape rs;
	rs.setSize(size);
	rs.setPosition(pos.x, pos.y);
	if (clickedDown)
		rs.setFillColor(Color::Green);
	else
		rs.setFillColor(Color::Blue);

	target->draw(rs);

	target->draw(text);
}