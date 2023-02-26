#ifndef __MAP_BROWSER_SCREEN_H__
#define __MAP_BROWSER_SCREEN_H__


#include <SFML/Graphics.hpp>

struct MainMenu;
struct MapBrowser;
struct MapBrowserHandler;
struct WorkshopMapPopup;

struct MapBrowserScreen
{
	MapBrowserHandler *browserHandler;
	//MapBrowser *browser;
	MainMenu *mainMenu;
	sf::Vertex bgQuad[4];

	//sf::Vertex downloadingQuad[4];

	MapBrowserScreen(MainMenu *mm);
	~MapBrowserScreen();

	void StartLocalBrowsing(int mode );
	void StartWorkshopBrowsing( int mode );
	void TurnOff();

	bool HandleEvent(sf::Event ev);
	
	void Update();
	bool IsCancelled();
	bool IsConfirmed();
	void Draw(sf::RenderTarget *target);
};

#endif