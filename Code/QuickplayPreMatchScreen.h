#ifndef __QUICKPLAY_PRE_MATCH_SCREEN_H__
#define __QUICKPLAY_PRE_MATCH_SCREEN_H__

#include <string>
#include <SFML/Graphics.hpp>
#include "GUI.h"
#include "steam/steam_api.h"
#include "Tileset.h"
#include "PlayerSkinShader.h"

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

	sf::Text mapNameText;
	sf::Text modeText;
	sf::Text leftPlayerName;
	sf::Text rightPlayerName;

	sf::Vertex bgQuad[4];

	sf::Vertex previewQuad[4];

	Tileset *ts_player0;
	Tileset *ts_player1;

	PlayerSkinShader p0Shader;
	PlayerSkinShader p1Shader;

	sf::Sprite p0Spr;
	sf::Sprite p1Spr;
	//sf::Vertex p0Quad[4];
	//sf::Vertex p1Quad[4];

	QuickplayPreMatchScreen();
	~QuickplayPreMatchScreen();
	void Update();
	void Draw(sf::RenderTarget *target);
	void Clear();
	void SetPreview(const std::string &previewPath);
	void SetAction(Action a);
	void ClearPreview();
	void SetToNetplayMatchParams();
};

#endif