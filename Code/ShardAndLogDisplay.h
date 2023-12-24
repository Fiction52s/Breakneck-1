#ifndef __SHARD_AND_LOG_DISPLAY_H__
#define __SHARD_AND_LOG_DISPLAY_H__

#include "Tileset.h"

struct Level;
struct GameSession;

struct ShardAndLogDisplay
{
	sf::Sprite shardIconSpr;
	sf::Text shardText;
	sf::Sprite logIconSpr;
	sf::Text logText;
	Tileset *ts_statIcons;
	sf::Vector2f topLeft;

	ShardAndLogDisplay( TilesetManager *tm );
	void SetTopLeft(sf::Vector2f pos);
	void SetLevel(Level *lev);
	void SetFromGame(GameSession *game );
	void Draw(sf::RenderTarget *target);
};

#endif