#ifndef __MAP_PUBLISH_FAILURE_POPUP_H__
#define __MAP_PUBLISH_FAILURE_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "steam/isteamremotestorage.h"

struct MapHeader;
struct LobbyParams;
struct MapNode;

struct MapPublishFailurePopup : GUIHandler, TilesetManager, PanelUpdater
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

	MapPublishFailurePopup();
	~MapPublishFailurePopup();
	void Activate(PublishedFileId_t uploadID, int errorCode, bool onCreate );
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);

	void ButtonCallback(Button *b, const std::string & e);
};


#endif
