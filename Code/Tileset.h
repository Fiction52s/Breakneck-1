#ifndef __TILESET_H__
#define __TILESET_H__

#include <SFML/Graphics.hpp>
#include <list>

struct Tileset
{
	~Tileset();
	sf::IntRect GetSubRect( int localID );
	sf::IntRect GetCustomSubRect(sf::Vector2i tileSize, sf::Vector2i origin, sf::Vector2i tileCount, int tileIndex);
	void SetSpriteTexture(sf::Sprite &spr);
	void SetSubRect(sf::Sprite &spr, int index,
		bool flipX = false, bool flipY = false );
	void SetQuadSubRect(sf::Vertex *v, int index,
		bool flipX = false, bool flipY = false);
	int GetNumTiles();
	int GetMemoryUsage();
	int GetMemoryTextureSize();
	sf::Texture * texture;
	int tileWidth;
	int tileHeight;
	std::string sourceName;
};

struct TilesetManager
{
	enum TilesetCategory : int
	{
		C_DEFAULT,
		C_PLAYER,
		C_ENV,
		C_FX,
		C_HUD,

		C_ENEMY, //this is where i start removing unused stuff
		C_TERRAIN,
		C_BACKGROUND,
		C_MOMENTA,
		C_SHARD,
		C_STORY,
		C_GOAL,
		C_ZONE,
		C_Count
	};

	TilesetManager();
	virtual ~TilesetManager();
	Tileset * GetSizedTileset(const std::string & s);
	Tileset * GetSizedTileset(const std::string &folder, const std::string & s);
	Tileset * GetTileset(const std::string & s, int tileWidth = 0, int tileHeight = 0);
	Tileset *GetUpdatedTileset(
		const std::string & s, int tileWidth, int tileHeight );
	Tileset *GetTileset(const std::string &s, sf::Texture *t);
	void ClearTilesets();
	int GetMemoryUsage();
	void DestroyTilesetIfExists(const std::string &sourceName,
		int altIndex = 0);
	void DestroyTileset(Tileset * t);
	void DestroyTilesetCategory(TilesetCategory tc);
	void ResetTilesetAccessCount();
	void CleanupUnusedTilests();
	void SetGameResourcesMode(bool on);
	bool IsResourceMode();

	/*sf::Texture *CreateAltColorTex( sf::Image &im,
		int numAltColors, 
		sf::Color *startColorBuf,
		sf::Color *endColorBuf );*/
	
	void SetParentTilesetManager(TilesetManager *man);

	bool lastQueriedTilesetWasDuplicate;
private:
	bool gameResourcesMode;
	TilesetManager *parentManager;
	TilesetCategory GetCategory(const std::string &s);
	Tileset *Create( TilesetCategory cat, const std::string &s, int tw, int th);
	void Add( TilesetCategory cat, const std::string &s, Tileset *ts);
	Tileset *Find( TilesetCategory cat, const std::string &s);
	//std::list<Tileset*> tilesetList;
	std::map<std::string,
		std::pair<Tileset*, int>> tilesetMaps[C_Count];
};

#endif