#ifndef __MAP_PUBLISH_POPUP_H__
#define __MAP_PUBLISH_POPUP_H__

#include "GUI.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct MapHeader;
struct MapNode;

struct MapPublishPopup : GUIHandler, TilesetManager, PanelUpdater
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

	Tileset *ts_preview;
	sf::Sprite previewSpr;
	TextBox *mapNameTextBox;
	TextBox *descriptionTextBox;

	MapPublishPopup();
	~MapPublishPopup();
	int GetLineWidth( int lineNumber );
	void Activate();
	void Update();
	void HandleEvent(sf::Event ev);
	void Draw(sf::RenderTarget *target);

	void ButtonCallback(Button *b, const std::string & e);
};

#endif