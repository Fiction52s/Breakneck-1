#ifndef __ONLINE_MENU_SCREEN_H__
#define __ONLINE_MENU_SCREEN_H__


#include <SFML/Graphics.hpp>
#include "GUI.h"
#include "Tileset.h"

struct MainMenu;
struct ControllerDualStateQueue;
struct ControlProfile;

struct OnlineMenuScreen : GUIHandler, TilesetManager
{
	enum Action
	{
		A_WORKSHOP,
		A_QUICKPLAY,
		A_CREATE_LOBBY,
		A_JOIN_LOBBY,
		Count,
		A_NONE,
		A_CANCELLED,
	};

	Action action;

	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];
	Panel *panel;
	Tileset *ts_bg;

	OnlineMenuScreen(MainMenu *mm);
	~OnlineMenuScreen();

	void ChooseRectEvent(ChooseRect *cr, int eventType);
	void ButtonCallback(Button *b, const std::string & e);
	void CancelCallback(Panel *p);

	bool HandleEvent(sf::Event ev);
	void Start();

	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif