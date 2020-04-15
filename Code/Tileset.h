#ifndef __TILESET_H__
#define __TILESET_H__

#include <SFML/Graphics.hpp>
#include <list>

struct Skin
{
	Skin(sf::Color *startCols, sf::Color *endCols,
		int p_numChanges, int p_index)
		:startColors(startCols), endColors(endCols),
		numChanges(p_numChanges), index(p_index)
	{

	}
	sf::Color *startColors;
	sf::Color *endColors;
	int numChanges;
	int index;
};

struct Tileset
{
	~Tileset();
	sf::IntRect GetSubRect( int localID );
	int GetNumTiles();
	sf::Texture * texture;
	int tileWidth;
	int tileHeight;
	std::string sourceName;
	int altColorIndex;
};

struct TilesetManager
{
	~TilesetManager();
	Tileset * GetTileset( const std::string & s, int tileWidth = 0, int tileHeight = 0,  int altColorIndex = 0 );
	Tileset * GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numColorChanges,
		sf::Color *startColorBuf, sf::Color *endColorBuf);
	Tileset * GetTileset(const std::string & s, int tileWidth, int tileHeight, Skin *skin);
	Tileset *GetUpdatedTileset(
		const std::string & s, int tileWidth, int tileHeight, int altColorIndex = 0);
	void ClearTilesets();
	void DestroyTileset(Tileset * t);

	sf::Texture *CreateAltColorTex( sf::Image &im,
		int numAltColors, 
		sf::Color *startColorBuf,
		sf::Color *endColorBuf );

private:
	std::list<Tileset*> tilesetList;
};

#endif