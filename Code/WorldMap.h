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
	MapSector(MapSelector *ms, int index);
	Tileset *ts_thumb;
	Tileset *ts_importantNodeIcons;
	
	sf::Text sectorNameText;
	int frame = 0;

	void Init(Sector *sec);
	Sector *sec;
	void UpdateNodePosition();
	//shard stuff
	int numLevels;
	void Load();

	
	SingleAxisSelector *saSelector;
	sf::Vertex levelCollectedShards[4 * 16];
	int selectedYIndex;
	sf::Sprite *topBonusNodes;
	sf::Sprite *botBonusNodes;
	sf::Sprite *nodes;
	sf::Sprite *paths;
	int sectorIndex;
	sf::Sprite thumbnail;
	sf::Vertex levelBG[4];
	sf::Vertex statsBG[4];
	sf::Vertex sectorStatsBG[4];
	sf::Text *unlockCondText;
	sf::Text shardsCollectedText;
	sf::Text completionPercentText;
	sf::Text levelPercentCompleteText;
	void Update(ControllerState &curr,
		ControllerState &prev);
	void SetXCenter( float x );
	void Draw(sf::RenderTarget *target);
	Tileset *ts_shards;
	MapSelector *ms;
	float xCenter;
	float percentComplete;
	void UpdateNodes();
	bool HasTopBonus(int node);
	bool HasBotBonus(int node);
	int GetNodeSubIndex(int node);
	int GetNodeBonusIndexTop(int node);
	int GetNodeBonusIndexBot(int node);
	void UpdateLevelStats();
	void DrawLevelStats(sf::RenderTarget *target);
	void DrawStats(sf::RenderTarget *target);
	void DrawUnlockConditions(sf::RenderTarget *target);
	void UpdateUnlockConditions();
	void UpdateStats();

	
	
};

struct MapSelector
{
	enum State
	{
		S_SLIDINGLEFT,
		S_SLIDINGRIGHT,
		S_IDLE,
	};

	State state;
	int slideDuration;
	MapSelector( MainMenu *mm, sf::Vector2f &pos );
	MapSector **sectors;
	
	
	sf::Vertex shoulderIcons[8];
	Tileset *ts_shoulderIcons;

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
	//sf::Sprite *nodes;
	//sf::Sprite *paths;
	//sf::Vertex *nodes;
	//sf::Vertex *paths;
	//sf::Vertex thumbnail[4];
	
	//sf::RectangleShape bottomBGRect;
	sf::Sprite bottomBG;
	sf::Sprite thumbnailBG;
	sf::Sprite shardBG;
	void Draw(sf::RenderTarget *target);
	SingleAxisSelector *saSelector;
	Tileset *ts_node;
	int worldIndex;

	int frame;
	//boost::thread *loadThread;
};


struct WorldMap
{
	enum State
	{
		SPACE,
		SPACE_TO_PLANET,
		PLANET,
		PlANET_TO_COLONY,
		COLONY,
		COLONY_TO_PLANET,
		PLANET_TO_SPACE,
	};

	sf::Shader zoomShader;
	WorldMap( MainMenu *mainMenu );
	void Reset( SaveFile *sf );
	~WorldMap();
	void Update(
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
	float currScale;
	sf::Vector2f currCenter;
	float oldZoomCurvePos;
	float zoomCurvePos;
	sf::View zoomView;
	sf::View uiView;

	Tileset *ts_space;
	Tileset *ts_planet;
	Tileset *ts_colonySelect;
	sf::Sprite colonySelectSprite;

	Tileset *ts_asteroid0;
	Tileset *ts_asteroid1;

	int selectedColony;
	
	Tileset *ts_colony[7];
	sf::Sprite colonySpr[7];
	sf::Sprite planetSpr;
	sf::Sprite spaceSpr;
	

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
	MainMenu *mainMenu;

	//ControllerState currInput;
	//ControllerState prevInput;
};

#endif