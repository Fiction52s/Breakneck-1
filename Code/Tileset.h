#ifndef __TILESET_H__
#define __TILESET_H__

#include <SFML/Graphics.hpp>
#include <list>

struct Tileset
{
	~Tileset();
	sf::IntRect GetSubRect( int localID );
	int GetNumTiles();
	sf::Texture * texture;
	int tileWidth;
	int tileHeight;
	std::string sourceName;
};

struct TilesetManager
{
	~TilesetManager();
	Tileset * GetTileset( const std::string & s, int tileWidth, int tileHeight );
	void ClearTilesets();
	std::list<Tileset*> tilesetList;
};

#endif