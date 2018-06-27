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

struct MapColony
{

};

struct MapSector
{
	enum State
	{
		NORMAL,
		LEVELJUSTCOMPLETE,
		LEVELCOMPLETEDWAIT,
		JUSTCOMPLETE,
		EXPLODECOMPLETE,
		COMPLETE
	};

	sf::Shader horizScrollShader1;
	sf::Shader horizScrollShader2;
	
	sf::Vertex backScrollEnergy[4];
	sf::Vertex frontScrollEnergy[4];

	Tileset *ts_scrollingEnergy;
	State state;
	int stateFrame;
	MapSector(MapSelector *ms, int index);
	~MapSector();
	Tileset *ts_thumb;
	Tileset *ts_importantNodeIcons;
	/*static void S_Init(Sector *m_sec,
		MapSector *ms );
	void ThreadedInit( Sector *m_sec );
	boost::thread *initThread;
	bool IsInitialized();*/
	sf::Text sectorNameText;
	int frame;
	//int *bossNumbers;
	int nodeSize;
	int pathLen;
	sf::Vector2f left;
	void Init(Sector *sec);
	Sector *sec;
	void UpdateNodePosition();
	//shard stuff
	int numLevels;
	void Load();
	Tileset *ts_node;
	int topUnlockedIndex;
	int botUnlockedIndex;
	int unlockedIndex;
	int unlockFrame;
	SingleAxisSelector *saSelector;
	//sf::Vertex *shardQuads;
	int numTotalShards;
	sf::Vertex *levelCollectedShards;
	sf::Vertex *levelCollectedShardsBG;
	int selectedYIndex;
	sf::Sprite *topBonusNodes;
	sf::Sprite *botBonusNodes;
	sf::Sprite *nodes;
	sf::Sprite nodeHighlight;
	void UpdateHighlight();
	int sectorIndex;
	sf::Sprite thumbnail;
	sf::Vertex levelBG[4];
	sf::Vertex statsBG[4];
	sf::Vertex sectorStatsBG[4];
	sf::Text *unlockCondText;
	int numUnlockConditions;
	sf::Text shardsCollectedText;
	sf::Text completionPercentText;
	sf::Text levelPercentCompleteText;
	sf::Sprite endSpr;

	Tileset *ts_energyCircle;
	Tileset *ts_energyTri;
	Tileset *ts_energyMask;
	Tileset *ts_nodeExplode;
	sf::Sprite nodeExplodeSpr;
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
	sf::Vector2f GetNodePos(int n);
	sf::Vector2f GetTopNodePos(int n);
	sf::Vector2f GetBotNodePos(int n);
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
	Tileset *ts_sectorLevelBG;
	Tileset *ts_levelStatsBG;
	Tileset *ts_sectorStatsBG;
	Tileset *ts_scrollingEnergy[7];
	
	sf::Vertex shoulderIcons[8];
	Tileset *ts_shoulderIcons;
	int numSectors;
	sf::Vector2f sectorCenter;
	int currSectorIndex;
	void UpdateAllInfo(int index);
	MainMenu *mainMenu;
	//int numNodeColumns;
	//int nodeSelectorWidth;
	sf::Vector2f centerPos;
	MapNodeState *nodeStates[3];
	void UpdateSprites();
	void Update(ControllerState &curr,
		ControllerState &prev);
	//sf::Sprite *nodes;
	
	//sf::Vertex *nodes;

	//sf::Vertex thumbnail[4];
	
	//sf::RectangleShape bottomBGRect;
	sf::Sprite bottomBG;
	sf::Sprite thumbnailBG;
	sf::Sprite shardBG;
	void Draw(sf::RenderTarget *target);
	SingleAxisSelector *saSelector;
	Tileset *ts_node[7];
	Tileset **ts_bossFight;
	Tileset *ts_sectorKey;
	Tileset **ts_sectorOpen;

	int frame;
	//boost::thread *loadThread;
};

struct WorldSelector
{
	WorldSelector::WorldSelector(MainMenu *mm);
	//void Reset()
	sf::Vertex quads[4 * 4];
	void Update();
	sf::Vector2f position;
	void SetPosition(sf::Vector2f &pos);
	void SetAlpha(float alpha);
	float angles[4];
	Tileset *ts;
	void Draw(sf::RenderTarget *target);
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

	WorldSelector *worldSelector;
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
	void SetDefaultSelections();
	sf::Sprite extraPassSpr;
	sf::Sprite selectorExtraPass;
	float currScale;
	sf::Vector2f currCenter;
	float oldZoomCurvePos;
	float zoomCurvePos;
	sf::View zoomView;
	sf::View uiView;

	Tileset *ts_space;
	Tileset *ts_planet;

	Tileset *ts_asteroids[4];
	sf::Vertex asteroidQuads[4*4];
	sf::Shader asteroidShader;
	int asteroidFrame;

	int selectedColony;
	
	Tileset *ts_colonyActiveZoomed[7];
	sf::Vertex worldActiveQuads[7 * 4];
	sf::Vertex worldActiveQuadsZoomed[7 * 4];
	Tileset *ts_colonyActive[7];
	Tileset *ts_colony[7];
	Tileset *ts_colonySelect;
	//Tileset *ts_colonySelect[7];
	//Tileset *ts_colonySelectZoomed[7];
	sf::Sprite colonySpr[7];
	sf::Sprite planetSpr;
	sf::Sprite spaceSpr;
	sf::Sprite colonySelectSpr;
	sf::Sprite colonySelectSprZoomed;
	

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
	MapSelector *CurrSelector();
	void InitSelectors();
	//MapSelector *testSelector;
	MapSelector *selectors[7];
	MainMenu *mainMenu;

	//ControllerState currInput;
	//ControllerState prevInput;
};

#endif