#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"
#include "UIMouse.h"

using namespace sf;
using namespace std;

ChooseRect::ChooseRect(ChooseRectIdentity ident, ChooseRectType crType,
	Vertex *v, sf::Vector2f &bSize, sf::Vector2f &p_pos, Panel *p)
	:PanelMember(p), quad(v), boxSize(bSize), pos(p_pos), chooseRectType(crType),
	rectIdentity(ident), circleMode(false)
{
	int alpha = 150;

	idleColor = Color::Black;
	idleColor.a = alpha;

	selectedIdleColor = Color::Magenta;
	selectedIdleColor.a = alpha;

	selectedMouseOverColor = Color::Red;
	selectedMouseOverColor.a = alpha;

	showName = true;

	selected = false;

	defaultIdleColor = idleColor;

	mouseOverColor = Color::Green;
	mouseOverColor.a = alpha;

	SetShown(false);

	focused = false;
	info = NULL;

	nameText.setFont(MainMenu::GetInstance()->arial);
	nameText.setCharacterSize(18);
	nameText.setFillColor(Color::White);
	nameText.setOutlineColor(Color::Black);
	nameText.setOutlineThickness(3);
	UpdateTextPosition();
}

void ChooseRect::SetInfo(void *inf)
{
	info = inf;
}

void ChooseRect::SetCircleMode(int p_radius)
{
	circleMode = true;
	circleRadius = p_radius;
}

void ChooseRect::SetRectMode()
{
	circleMode = false;
}

void ChooseRect::SetIdleColor(sf::Color c)
{
	idleColor = c;
}


void ChooseRect::SetName(const std::string &name)
{
	nameText.setString(name);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		0);
}

void ChooseRect::SetNameSize(int charHeight)
{
	nameText.setCharacterSize(charHeight);
	nameText.setOrigin(nameText.getLocalBounds().left + nameText.getLocalBounds().width / 2,
		0);
}

int ChooseRect::GetNameSize()
{
	return nameText.getCharacterSize();
}

void ChooseRect::ShowName(bool show)
{
	showName = show;
}

void ChooseRect::Init()
{
	SetRectColor(quad, idleColor);
	SetSize(boxSize);
}

void ChooseRect::UpdateTextPosition()
{
	nameText.setPosition(Vector2f(pos.x + boxSize.x / 2, pos.y));// +boxSize + 2));
}

void ChooseRect::SetPosition(sf::Vector2f &p_pos)
{
	pos = p_pos;
	UpdateRectDimensions();
	UpdateTextPosition();
}

void ChooseRect::SetSize(sf::Vector2f &bSize)
{
	boxSize = bSize;
	UpdateRectDimensions();
}

void ChooseRect::UpdateRectDimensions()
{
	//SetRectCenter(quad, boxSize, boxSize, pos);
	bounds.left = pos.x;// -boxSize / 2.f;
	bounds.top = pos.y;// -boxSize / 2.f;
	bounds.width = boxSize.x;
	bounds.height = boxSize.y;
}

void ChooseRect::SetShown(bool s)
{
	if (!s && show)
	{
		SetRectTopLeft(quad, 0, 0, Vector2f(0, 0));
	}
	else if (s && !show)
	{
		if (circleMode)
		{
			SetRectTopLeft(quad, 0, 0, Vector2f(0, 0));
		}
		else
		{
			SetRectTopLeft(quad, boxSize.x, boxSize.y, pos);
		}
		SetSize(boxSize);
	}
	show = s;
}

sf::Vector2f ChooseRect::GetGlobalPos()
{
	//return mouseUser->GetFloatPos() + pos;
	return Vector2f(panel->pos) + pos;
}

sf::Vector2f ChooseRect::GetGlobalCenterPos()
{
	return GetGlobalPos() + Vector2f(boxSize.x / 2, boxSize.y / 2);
}

void ChooseRect::SetActive(bool a)
{
	active = a;
}

void ChooseRect::Deactivate()
{
	focused = false;
	selected = false;
	SetRectColor(quad, idleColor);
}

void ChooseRect::Select()
{
	selected = true;

	if (focused)
	{
		SetRectColor(quad, selectedMouseOverColor);
	}
	else
	{
		SetRectColor(quad, selectedIdleColor);
	}
}

void ChooseRect::Deselect()
{
	selected = false;

	if (focused)
	{
		SetRectColor(quad, mouseOverColor);
	}
	else
	{
		SetRectColor(quad, idleColor);
	}
}

bool ChooseRect::ContainsPoint(sf::Vector2i &mousePos)
{
	if (circleMode)
	{
		Vector2i center(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
		Vector2f diff(mousePos - center);
		return (length(diff) <= circleRadius);
	}
	else
	{
		return bounds.contains(mousePos);
	}
}

bool ChooseRect::MouseUpdate()
{
	if (hidden)
		return false;

	if (!show)
	{
		return false;
	}

	Vector2i mousePos = panel->GetMousePos();

	if (MOUSE.IsMouseLeftClicked())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_LEFTCLICKED);
			focused = true;
		}
	}
	else if (MOUSE.IsMouseLeftReleased())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_LEFTRELEASED);
			focused = true;
		}
	}
	else
	{
		if (ContainsPoint(mousePos))
		{
			if (selected)
			{
				SetRectColor(quad, selectedMouseOverColor);
			}
			else
			{
				SetRectColor(quad, mouseOverColor);
			}
			
			if (!focused)
			{
				panel->handler->ChooseRectEvent(this, E_FOCUSED);
			}
			focused = true;
		}
		else
		{
			if (selected)
			{
				SetRectColor(quad, selectedIdleColor);
			}
			else
			{
				SetRectColor(quad, idleColor);
			}
			
			if (focused)
			{
				panel->handler->ChooseRectEvent(this, E_UNFOCUSED);
			}
			focused = false;

			//Unfocus();
		}
	}

	if (MOUSE.IsMouseRightClicked())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_RIGHTCLICKED);
			//focused = true;
		}
	}
	else if (MOUSE.IsMouseRightReleased())
	{
		if (ContainsPoint(mousePos))
		{
			panel->handler->ChooseRectEvent(this, E_RIGHTRELEASED);
			//focused = true;
		}
	}

	return true;
}

EnemyChooseRect *ChooseRect::GetAsEnemyChooseRect()
{
	if (chooseRectType == ENEMY)
	{
		return (EnemyChooseRect*)this;
	}
	else
	{
		return NULL;
	}
}

ImageChooseRect *ChooseRect::GetAsImageChooseRect()
{
	if (chooseRectType == IMAGE)
	{
		return (ImageChooseRect*)this;
	}
	else
	{
		return NULL;
	}
}
