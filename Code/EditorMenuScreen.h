#ifndef __EDITOR_MENU_SCREEN_H__
#define __EDITOR_MENU_SCREEN_H__


#include <SFML/Graphics.hpp>
#include "GUI.h"
#include "Tileset.h"

struct MainMenu;
struct MapNode;
struct MapBrowserScreen;

struct EditorMenuScreen : GUIHandler, TilesetManager
{
	enum Action
	{
		A_NEW_MAP,
		A_OPEN_MAP,
		A_CHOOSE_MAP,
		Count,
		A_NONE,
		A_CANCELLED,
	};

	Action action;

	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];
	Panel *panel;
	Tileset *ts_bg;
	Tileset *ts_title;
	MapBrowserScreen * mapBrowserScreen;
	sf::Sprite titleSpr;
	Tileset *ts_newFile;
	Tileset *ts_openFile;

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