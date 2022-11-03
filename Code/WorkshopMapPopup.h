#ifndef __WORKSHOP_MAP_POPUP_H__
#define __WORKSHOP_MAP_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>

struct MapHeader;
struct LobbyParams;
struct MapNode;

struct WorkshopMapPopup : GUIHandler
{
	enum Action
	{
		A_INACTIVE,
		A_ACTIVE,
		A_HOST,
		A_EDIT,
		A_PLAY,
		A_SAVE,
		A_BACK,
	};

	Panel *panel;
	bool active;
	Action action;

	sf::Sprite previewSpr;
	Tileset *ts_preview;


	MapHeader *currMapHeader;
	int chosenGameModeType;
	std::string mapPath;

	sf::Vector2i previewPos;
	HyperLink *creatorLink;

	sf::Text *creatorLabel;
	sf::Text *creatorByLabel;
	HyperLink *nameLink;
	sf::Text *nameLabel;
	sf::Text *descriptionText;
	MapNode *currMapNode;

	HyperLink *fileLink;

	Button *editLocalCopyButton;
	Button *playButton; //hitting this will take you to freeplay mode with this map selected.
	Button *hostButton;
	Button *saveLocalCopyButton;
	Button *backButton;
	Button *subscribeButton;
	Button *unsubscribeButton;

	WorkshopMapPopup();
	~WorkshopMapPopup();
	
	void CheckSubscription();
	void CheckHeader();
	bool Activate(MapNode *mp);
	void Deactivate();
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);

	void ButtonCallback(Button *b, const std::string & e);
	void PanelCallback(Panel *p, const std::string & e);
};

#endif