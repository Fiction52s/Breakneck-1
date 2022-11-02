#include "GUI.h"
#include <ShlObj.h> //for opening an explorer window
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
	linkType = LINK_WEBPAGE;
	linkURL = p_linkURL;
	characterHeight = charHeight;
	text.setString(t);
	text.setStyle(sf::Text::Underlined);
	text.setFont(f);
	text.setFillColor(Color::White);
	text.setCharacterSize(characterHeight);

	SetPos(pos);
}

void HyperLink::Deactivate()
{
	clickedDown = false;
}

void HyperLink::SetLinkURL( const std::string &url )
{
	linkURL = url;
	linkType = LINK_WEBPAGE;
}

void HyperLink::SetLinkFileAndFolder(const std::string &fileStr, const std::string folderStr)
{
	file = fileStr;
	folder = folderStr;
	linkType = LINK_FILE;
}

void HyperLink::SetString(const std::string &str)
{
	text.setString(str);
}

void HyperLink::SetPos(sf::Vector2i &p_pos)
{
	pos = p_pos;
	//text.setPosition(pos.x + size.x / 2, pos.y + size.y / 2);
	text.setPosition(pos.x, pos.y);
}

//#include <ShlObj.h> //for opening an explorer window
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

			if (linkType == LINK_WEBPAGE)
			{
				SteamFriends()->ActivateGameOverlayToWebPage(linkURL.c_str());
			}
			else if( linkType == LINK_FILE )
			{
#ifdef _WIN32
//#include <ShlObj.h> //for opening an explorer window
				ITEMIDLIST *dir = ILCreateFromPath(folder.c_str());
				LPITEMIDLIST *items = new LPITEMIDLIST[1];
				items[0] = ILCreateFromPath(file.c_str());
				SHOpenFolderAndSelectItems(dir, 1, (LPCITEMIDLIST*)items, 0);
				ILFree(items[0]);
				ILFree(dir);
				delete[] items;
#endif
			}
			else
			{
				assert(0);
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