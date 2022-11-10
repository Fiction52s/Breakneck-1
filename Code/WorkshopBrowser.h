#ifndef __WORKSHOP_BROWSER_H__
#define __WORKSHOP_BROWSER_H__

#include <SFML/Graphics.hpp>
#include <steam/steam_api.h>

struct WorkshopMapPopup;
struct MapBrowserScreen;
struct SavePopup;
struct MessagePopup;

struct WorkshopBrowser
{
	enum Action
	{
		A_IDLE,
		A_BROWSER,
		A_POPUP,
		A_SAVE_POPUP,
		A_SAVE_MESSAGE,
		A_BACK,
	};

	WorkshopMapPopup *workshopMapPopup;
	SavePopup *savePopup;
	MessagePopup *saveMessagePopup;
	MapBrowserScreen *mapBrowserScreen;
	int action;
	int frame;
	bool editAfterSaving;
	std::string savedPath;
	

	WorkshopBrowser();
	~WorkshopBrowser();
	void ClearAllPreviewsButSelected();
	void RequestPreviews();
	void Start();
	void Quit();
	void HandleEvent(sf::Event ev);
	void Update();
	void Draw(sf::RenderTarget *target);
};

#endif