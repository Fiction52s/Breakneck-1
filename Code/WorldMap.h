#ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

#include <SFML/Graphics.hpp>
#include <boost/filesystem.hpp>
#include "LevelSelector.h"
#include "Input.h"
#include "Tileset.h"
#include "ItemSelector.h"

struct MainMenu;
struct SaveFile;
struct MapSelector;
struct MapSector;

enum MapNodeState
{
	NS_DEACTIVATED,
	NS_ACTIVATED,
	NS_PARTIALLY_BEATEN,
	NS_FULLY_BEATEN
};

struct SectorNode
{
	//shard info
	//completion info
};

struct Sector;
struct MapSector
{
	MapSector(MapSelector *ms);
	Tileset *ts_thumb;
	
	void Init(Sector *sec);
	Sector *sec;
	void UpdateNodePosition();
	//shard stuff
	int numLevels;
	void Load();
	SingleAxisSelector *saSelector;
	int selectedYIndex;
	sf::Vertex *nodes;
	sf::Vertex *paths;
	void Update(ControllerState &curr,
		ControllerState &prev);
	void Draw(sf::RenderTarget *target);
	MapSelector *ms;
	void UpdateNodes();
	bool HasTopBonus(int node);
	bool HasBotBonus(int node);
	int GetNodeSubIndex(int node);
	int GetNodeBonusIndexTop(int node);
	int GetNodeBonusIndexBot(int node);
	
};

struct MapSelector
{
	MapSelector( MainMenu *mm, sf::Vector2f &pos );
	MapSector **sectors;
	int numSectors;
	sf::Vector2f sectorCenter;
	int currSectorIndex;
	void UpdateAllInfo();
	MainMenu *mainMenu;
	//int numNodeColumns;
	//int nodeSelectorWidth;
	sf::Vector2f centerPos;
	MapNodeState *nodeStates[3];
	void UpdateSprites();
	void Update(ControllerState &curr,
		ControllerState &prev);
	sf::Vertex *nodes;
	sf::Vertex *paths;
	//sf::Vertex thumbnail[4];
	sf::Sprite thumbnail;
	//sf::RectangleShape bottomBGRect;
	sf::Sprite bottomBG;
	sf::Sprite thumbnailBG;
	sf::Sprite shardBG;
	void Draw(sf::RenderTarget *target);
	SingleAxisSelector *saSelector;
	Tileset *ts_node;
	bool sectorSelected;
	int worldIndex;
};


struct WorldMap
{
	enum State
	{
		OFF,
		PLANET_AND_SPACE,
		PLANET_TRANSITION,
		PLANET,
		SECTION_TRANSITION,
		SECTION, 
		COLONY_TRANSITION,
		COLONY
	};

	sf::Shader zoomShader;
	WorldMap( MainMenu *mainMenu );
	void Reset( SaveFile *sf );
	~WorldMap();
	bool Update(
		ControllerState &prevInput,
		ControllerState &currInput );
	void Draw( sf::RenderTarget *target );
	void UpdateMapList();
	const std::string & GetSelected();
	void UpdateMapList( TreeNode *parentNode, const std::string &relativePath );
	void ClearEntries();
	void UpdateColonySelect();
	int Tex( int index, int level, TreeNode *entry );
	State state;
	int frame;
	sf::Sprite extraPassSpr;
	sf::Sprite zoomedMapSpr;
	Tileset *ts_zoomedMapw1;
	float currScale;
	sf::Vector2f currCenter;
	float oldZoomCurvePos;
	float zoomCurvePos;
	sf::View zoomView;
	sf::View uiView;

	Tileset *ts_planetAndSpace;
	Tileset *ts_planet;
	Tileset *ts_colonySelect;
	sf::Sprite colonySelectSprite;
	//sf::Texture *planetAndSpaceTex;
	//sf::Texture *planetTex;
	int selectedColony;
	
	//sf::Texture *sectionTex[6];
	//sf::Texture *colonyTex[6];
	Tileset *ts_section[6];
	//Tileset *ts_colony[6];

	Tileset *ts_colony[7];
	sf::Sprite back;
	sf::Sprite front;

	TreeNode **dirNode;
	TreeNode *entries;
	int selectedLevel;
	int numTotalEntries;
	sf::Text * text;
	sf::Font font;
	std::string * localPaths;
	int fontHeight;
	int leftBorder;
	//int xspacing;
	int yspacing;
	sf::Vector2f menuPos;
	sf::RectangleShape bgRect;
	sf::RectangleShape selectedRect;

	bool moveDown;
	bool moveUp;

	MapSelector *testSelector;

	//ControllerState currInput;
	//ControllerState prevInput;
};

#endif