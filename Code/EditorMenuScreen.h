#ifndef __EDITOR_MENU_SCREEN_H__
#define __EDITOR_MENU_SCREEN_H__


#include <SFML/Graphics.hpp>
#include "GUI.h"

struct MainMenu;

struct EditorMenuScreen : GUIHandler
{
	enum Action
	{
		A_NEW_MAP,
		A_OPEN_MAP,
		Count,
		A_NONE,
		A_CANCELLED,
	};

	Action action;

	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];
	Panel *panel;

	EditorMenuScreen(MainMenu *mm);
	~EditorMenuScreen();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void CancelCallback(Panel *p);

	bool HandleEvent(sf::Event ev);
	void Start();

	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif