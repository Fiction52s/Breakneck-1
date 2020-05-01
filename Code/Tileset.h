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
	void SetSpriteTexture(sf::Sprite &spr);
	void SetSubRect(sf::Sprite &spr, int index,
		bool flipX = false, bool flipY = false );
	void SetQuadSubRect(sf::Vertex *v, int index,
		bool flipX = false, bool flipY = false);
	int GetNumTiles();
	sf::Texture * texture;
	int tileWidth;
	int tileHeight;
	std::string sourceName;
	int altColorIndex;
};

struct TilesetManager
{
private:
	enum TilesetCategory : int
	{
		C_DEFAULT,
		C_PLAYER,
		C_ENEMY,
		C_TERRAIN,
		C_ENV,
		C_BACKGROUND,
		C_MOMENTA,
		C_SHARD,
		C_STORY,
		C_GOAL,
		C_ZONE,
		C_FX,
		C_HUD,
		C_Count
	};
public:
	~TilesetManager();
	Tileset * GetSizedTileset(const std::string & s, int altColorIndex = 0);
	Tileset * GetSizedTileset(const std::string &folder, const std::string & s, int altColorIndex = 0);
	Tileset * GetSizedTileset(const std::string &folder, const std::string & s, Skin *skin);
	Tileset * GetTileset(const std::string & s, int tileWidth = 0, int tileHeight = 0, int altColorIndex = 0);
	Tileset * GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numColorChanges,
		sf::Color *startColorBuf, sf::Color *endColorBuf);
	Tileset * GetSizedTileset(const std::string & s, int altColorIndex, int numColorChanges,
		sf::Color *startColorBuf, sf::Color *endColorBuf);
	Tileset * GetTileset(const std::string & s, int tileWidth, int tileHeight, Skin *skin);
	Tileset *GetUpdatedTileset(
		const std::string & s, int tileWidth, int tileHeight, int altColorIndex = 0);
	void ClearTilesets();
	void DestroyTileset(Tileset * t);
	void ResetTilesetAccessCount();
	void CleanupUnusedTilests();

	sf::Texture *CreateAltColorTex( sf::Image &im,
		int numAltColors, 
		sf::Color *startColorBuf,
		sf::Color *endColorBuf );

private:
	TilesetCategory GetCategory(const std::string &s);
	Tileset *Create( TilesetCategory cat, const std::string &s, int tw, int th,
		int altColorIndex );
	void Add( TilesetCategory cat, const std::string &s, Tileset *ts);
	Tileset *Find( TilesetCategory cat, const std::string &s, int altColorIndex);
	//std::list<Tileset*> tilesetList;
	std::map<std::string, 
		std::list<
		std::pair<Tileset*,int>>> tilesetMaps[C_Count];
};

#endif