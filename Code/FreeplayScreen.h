#ifndef __FREEPLAY_SCREEN_H__
#define __FREEPLAY_SCREEN_H__


#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;
struct WorkshopMapPopup;

struct FreeplayScreen : TilesetManager
{
	enum Action
	{
		A_WAITING_FOR_PLAYERS,
		A_READY,
	};

	int action;
	int frame;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];
	//sf::Vertex bgQuad[4];

	FreeplayScreen(MainMenu *mm);
	~FreeplayScreen();

	void Start();
	void Quit();
	bool HandleEvent(sf::Event ev);

	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif