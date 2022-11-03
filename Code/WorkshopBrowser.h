#ifndef __WORKSHOP_BROWSER_H__
#define __WORKSHOP_BROWSER_H__

#include <SFML/Graphics.hpp>

struct WorkshopMapPopup;
struct MapBrowserScreen;

struct WorkshopBrowser
{
	enum Action
	{
		A_IDLE,
		A_BROWSER,
		A_POPUP,
		A_BACK,
	};

	WorkshopMapPopup *workshopMapPopup;
	MapBrowserScreen *mapBrowserScreen;
	int action;
	int frame;

	WorkshopBrowser();
	~WorkshopBrowser();
	void Start();
	void Quit();
	void HandleEvent(sf::Event ev);
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif