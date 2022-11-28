#ifndef __MENU_POPUP_H__
#define __MENU_POPUP_H__

#include <SFML\Graphics.hpp>
#include "Input.h"
#include "GUI.h"

struct MainMenu;


struct MenuInfoPopup : GUIHandler
{
	enum Action
	{
		A_CLOSED,
		A_OPEN,
	};

	Action action;
	Panel *panel;
	Button *okButton;
	sf::Vector2f size;
	Label *infoLabel;
	sf::Vector2f position;
	//sf::Vertex popupBGQuad[4];

	int forcedStayOpenFrames;

	MenuInfoPopup(MainMenu*mainMenu);
	~MenuInfoPopup();
	void SetText(const std::string &str);
	void Pop(const std::string &str, int forcedStayOpenFrames = 0);
	bool Update(ControllerState &currInput,
		ControllerState &prevInput);
	void SetPos(sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
	void HandleEvent(sf::Event ev);
	void ButtonCallback(Button *b, const std::string & e);
};

#endif