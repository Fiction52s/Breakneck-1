#ifndef __MENU_POPUP_H__
#define __MENU_POPUP_H__

#include <SFML\Graphics.hpp>
#include "Input.h"

struct MainMenu;


struct MenuInfoPopup
{
	sf::Vector2f size;
	sf::Text text;
	sf::Vector2f position;
	sf::Vertex popupBGQuad[4];

	int forcedStayOpenFrames;

	MenuInfoPopup(MainMenu*mainMenu);
	void SetText(const std::string &str);
	void Pop(const std::string &str, int forcedStayOpenFrames = 0);
	bool Update(ControllerState &currInput,
		ControllerState &prevInput);
	void SetPos(sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
};

#endif