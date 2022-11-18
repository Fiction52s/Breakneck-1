#ifndef __MAP_POST_PUBLISH_POPUP_H__
#define __MAP_POST_PUBLISH_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Tileset.h"
#include "steam/isteamremotestorage.h"

struct MapHeader;
struct MapNode;

struct MapPostPublishPopup : GUIHandler, TilesetManager, PanelUpdater
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

	MapPostPublishPopup();
	~MapPostPublishPopup();
	void Activate( bool agreementSigned, PublishedFileId_t uploadID );
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);

	void ButtonCallback(Button *b, const std::string & e);
};


#endif
