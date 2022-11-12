#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"
#include "UIMouse.h"

using namespace sf;
using namespace std;


const int CHECKBOXSIZE = 32;

CheckBox::CheckBox(const std::string &n, int posx, int posy, Panel *p)
	:PanelMember(p), pos(posx, posy), clickedDown(false), name(n), checked(false)
{
	size = Vector2i(CHECKBOXSIZE, CHECKBOXSIZE);
}

void CheckBox::Deactivate()
{
	clickedDown = false;
}

void CheckBox::SetLockedStatus(bool check, bool lock)
{
	checked = check;
	locked = lock;
}

bool CheckBox::MouseUpdate()
{
	if (hidden)
		return false;

	Vector2i mousePos = panel->GetMousePos();
	sf::Rect<int> r(pos.x, pos.y, CHECKBOXSIZE, CHECKBOXSIZE);
	bool containsMouse = r.contains(mousePos);
	UpdateToolTip(containsMouse);

	if (locked)
		return false;

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
			checked = !checked;
			if (checked)
			{
				panel->SendEvent(this, "checked");
			}
			else
			{
				panel->SendEvent(this, "unchecked");
			}

			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}


//void CheckBox::SetPanelPos(const sf::Vector2i &p_pos)
//{
//
//}

void CheckBox::Draw(RenderTarget *target)
{
	if (hidden)
		return;

	sf::RectangleShape rs;
	rs.setSize(sf::Vector2f(CHECKBOXSIZE, CHECKBOXSIZE));
	rs.setPosition(pos.x, pos.y);

	if (locked)
	{
		if (checked)
		{
			rs.setFillColor(Color(255, 100, 100));
		}
		else
		{
			rs.setFillColor(Color(100, 100, 100));
		}
	}
	else
	{
		if (clickedDown)
		{
			rs.setFillColor(Color::Cyan);
		}
		else
		{
			if (checked)
			{
				rs.setFillColor(Color::Magenta);
			}
			else
			{
				rs.setFillColor(Color::Black);
			}
		}
	}

	target->draw(rs);
}