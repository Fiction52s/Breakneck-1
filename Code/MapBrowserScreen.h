#ifndef __MAP_BROWSER_SCREEN_H__
#define __MAP_BROWSER_SCREEN_H__


#include <SFML/Graphics.hpp>

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;

struct MapBrowserScreen
{
	MapBrowserHandler *browserHandler;
	//MapBrowser *browser;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];

	//sf::Vertex downloadingQuad[4];

	MapBrowserScreen(MainMenu *mm);
	~MapBrowserScreen();

	void StartLocalBrowsing();
	void StartWorkshopBrowsing();

	bool HandleEvent(sf::Event ev);
	
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif