#include "GUI.h"
#include <assert.h>
#include <iostream>
#include "Session.h"
#include "EditorDecorInfo.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

HyperLink::HyperLink(const std::string &n, int posx, int posy, int charHeight, sf::Font &f, const std::string & t, const std::string &p_linkURL, Panel *p)
	:PanelMember(p), pos(posx, posy), clickedDown(false), hoveredOver(false), name(n)
{
	linkURL = p_linkURL;
	characterHeight = charHeight;
	text.setString(t);
	text.setStyle(sf::Text::Underlined);
	text.setFont(f);
	text.setFillColor(Color::White);
	text.setCharacterSize(characterHeight);

	//auto bounds = text.getLocalBounds();
	//text.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);

	SetPos(pos);
}

void HyperLink::Deactivate()
{
	clickedDown = false;
}

void HyperLink::SetLinkURL( const std::string &url )
{
	linkURL = url;
}

void HyperLink::SetPos(sf::Vector2i &p_pos)
{
	pos = p_pos;
	//text.setPosition(pos.x + size.x / 2, pos.y + size.y / 2);
	text.setPosition(pos.x, pos.y);
}


bool HyperLink::MouseUpdate()
{
	if (hidden)
		return false;

	Vector2i mousePos = panel->GetMousePos();
	//sf::Rect<int> r(pos.x, pos.y, size.x, size.y);

	sf::FloatRect r = text.getGlobalBounds();

	bool containsMouse = r.contains(Vector2f(mousePos));
	UpdateToolTip(containsMouse);

	hoveredOver = containsMouse;

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

			SteamFriends()->ActivateGameOverlayToWebPage(linkURL.c_str());//"https://steamcommunity.com/sharedfiles/workshoplegalagreement");
			return true;
		}
		else
		{
			clickedDown = false;
		}
	}

	return false;
}

void HyperLink::Draw(RenderTarget *target)
{
	if (hidden)
		return;

	if (!clickedDown && !hoveredOver)
	{
		text.setFillColor(Color::White);
	}
	else if (!clickedDown)
	{
		text.setFillColor(Color::Cyan);
	}
	else
	{
		text.setFillColor(Color::Magenta);
	}

	target->draw(text);
}