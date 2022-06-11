#ifndef __MAP_BROWSER_SCREEN_H__
#define __MAP_BROWSER_SCREEN_H__


#include <SFML/Graphics.hpp>

struct MainMenu;
struct MapBrowser;
struct DefaultMapBrowserHandler;

struct MapBrowserScreen
{
	DefaultMapBrowserHandler *browserHandler;
	//MapBrowser *browser;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];

	MapBrowserScreen(MainMenu *mm);
	~MapBrowserScreen();

	void Start();
	
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif