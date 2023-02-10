#ifndef __QUICKPLAY_PRE_MATCH_SCREEN_H__
#define __QUICKPLAY_PRE_MATCH_SCREEN_H__

#include <string>
#include <SFML/Graphics.hpp>
#include "GUI.h"
#include "steam/steam_api.h"
#include "Tileset.h"

struct WaitingRoom;

struct MapHeader;

struct QuickplayPreMatchScreen : TilesetManager
{
	enum Action
	{
		A_STARTUP,
		A_SHOW,
		A_DONE,
	};

	int action;
	int frame;

	MapHeader *mapHeader;

	CSteamID ownerID;

	Tileset *ts_preview;

	sf::Text nameText;
	sf::Text descriptionText;

	sf::Vertex bgQuad[4];

	sf::Vertex previewQuad[4];
	sf::Vector2f previewBottomLeft;

	QuickplayPreMatchScreen();
	~QuickplayPreMatchScreen();
	void Update();
	void Draw(sf::RenderTarget *target);
	void Clear();
	void SetPreview(const std::string &previewPath);
	void SetAction(Action a);
	void ClearPreview();
	void UpdateMapHeader(const std::string &mapPath);
};

#endif