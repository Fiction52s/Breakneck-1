#ifndef __TRANSFER_LOADING_SCREEN_H__
#define __TRANSFER_LOADING_SCREEN_H__

struct Level;
struct ShardAndLogDisplay;

#include <SFML\Graphics.hpp>
#include "Tileset.h"

struct TransferLoadingScreen : TilesetManager
{
	sf::Text levelNameText;
	ShardAndLogDisplay *shardAndLogDisplay;
	int frame;
	int action;
	Level *level;
	Tileset *ts_mapPreview;
	sf::Sprite mapPreviewSpr;

	TransferLoadingScreen();
	virtual ~TransferLoadingScreen();
	void SetLevel( Level *lev );
	void DrawLevelInfo(sf::RenderTarget *target);
	void DrawMapPreview(sf::RenderTarget *target);
	void DestroyMapPreview();
	void UpdateMapPreview();
};

#endif