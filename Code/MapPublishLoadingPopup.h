#ifndef __MAP_PUBLISH_LOADING_POPUP_H__
#define __MAP_PUBLISH_LOADING_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "steam/isteamremotestorage.h"

struct MapHeader;
struct MapNode;

struct MapPublishLoadingPopup : GUIHandler, TilesetManager, PanelUpdater
{
	enum Action
	{
		A_INACTIVE,
		A_ACTIVE,
		A_CONFIRMED,
		A_CANCELLED,
	};

	Panel *panel;
	bool active;
	Action action;
	sf::Text *mapNameText;
	Tileset *ts_preview;
	sf::Sprite previewSpr;

	HyperLink *mapLink;
	HyperLink *agreementLink;
	sf::Text *remainPrivateText;

	int frame;
	int currAttempt;
	int maxAttempts;

	MapPublishLoadingPopup();
	~MapPublishLoadingPopup();
	void UpdateFrame(int numUpdateFrames);
	void Activate();
	void SetAttempt(int currAttempt, int maxAttempts);
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);
	void ClosePopup();

	void ButtonCallback(Button *b, const std::string & e);
};


#endif
