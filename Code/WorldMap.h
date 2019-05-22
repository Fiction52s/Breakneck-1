 #ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

#include <SFML/Graphics.hpp>
#include <boost/filesystem.hpp>
#include "LevelSelector.h"
#include "Input.h"
#include "Tileset.h"
#include "ItemSelector.h"
#include "SaveFile.h"

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

struct MapNode
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

	MapSector(MapSelector *ms, int index);
	~MapSector();
	void UpdateUnlockedLevelCount();
	int unlockedLevelCount;

	int frame;
	int stateFrame;
	State state;

	bool IsFocused();
	void RunSelectedMap();

	
	Tileset *ts_thumb;
	sf::Text sectorNameText;
	
	int nodeSize;
	int pathLen;
	sf::Vector2f left;
	void Init(Sector *sec);
	Sector *sec;

	int GetNumLevels();

	void UpdateNodePosition();
	int GetSelectedIndex();
	Level &GetSelectedLevel();


	int numLevels;
	void Load();
	int unlockedIndex;
	int unlockFrame;
	//SingleAxisSelector *saSelector; //select level

	int numTotalShards;
	sf::Vertex *levelCollectedShards;
	sf::Vertex *levelCollectedShardsBG;

	sf::Sprite *nodes;

	//worry about bonuses later
	int selectedYIndex;
	sf::Sprite *topBonusNodes;
	sf::Sprite *botBonusNodes;
	
	
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
	Tileset *ts_shards;

	sf::Sprite nodeExplodeSpr;

	void Update(ControllerState &curr,
		ControllerState &prev);
	void SetXCenter( float x );
	void Draw(sf::RenderTarget *target);
	
	
	
	void UpdateNodes();
	sf::Vector2f GetNodePos(int n);
	sf::Vector2f GetSelectedNodePos();
	int GetNodeSubIndex(int node);
	int GetSelectedNodeSubIndex();

	int GetSelectedNodeBossFightType();

	bool HasTopBonus(int node);
	bool HasBotBonus(int node);
	sf::Vector2f GetTopNodePos(int n);
	sf::Vector2f GetBotNodePos(int n);
	int GetNodeBonusIndexTop(int node);
	int GetNodeBonusIndexBot(int node);
	
	float xCenter;
	float percentComplete;
	MapSelector *ms;
	
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
		//S_SLIDINGLEFT,
		//S_SLIDINGRIGHT,
		S_IDLE,
	};

	sf::Sprite nodeHighlight;
	void UpdateHighlight();

	sf::Shader horizScrollShader1;
	sf::Shader horizScrollShader2;

	sf::Vertex backScrollEnergy[4];
	sf::Vertex frontScrollEnergy[4];
	//Tileset *ts_scrollingEnergy;

	
	void RunSelectedMap();
	State state;
	//int slideDuration;
	MapSelector( MainMenu *mm, sf::Vector2f &pos, int wIndex );
	int worldIndex;
	~MapSelector();
	MapSector **sectors;
	Tileset *ts_sectorLevelBG;
	Tileset *ts_levelStatsBG;
	Tileset *ts_sectorStatsBG;
	Tileset *ts_scrollingEnergy;
	MapSector *GetFocusedSector();

	int numSectors;
	
	//int currSectorIndex;
	sf::Vector2f sectorCenter;
	
	void UpdateAllInfo(int index);
	MainMenu *mainMenu;

	sf::Vector2f centerPos;
	MapNodeState *nodeStates[3];
	void UpdateSprites();
	void Update(ControllerState &curr,
		ControllerState &prev);

	sf::Sprite bottomBG;
	sf::Sprite thumbnailBG;
	sf::Sprite shardBG;
	void Draw(sf::RenderTarget *target);
	SingleAxisSelector *sectorSelector;
	SingleAxisSelector *mapSelector;
	Tileset *ts_node;
	Tileset **ts_bossFight;
	Tileset *ts_sectorKey;
	Tileset **ts_sectorOpen;
	int frame;
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
		START_LEVEL
	};


	void RunSelectedMap();
	WorldSelector *worldSelector;
	sf::Shader zoomShader;
	WorldMap( MainMenu *mainMenu );
	void Reset( SaveFile *sf );
	~WorldMap();
	void Update(
		ControllerState &prevInput,
		ControllerState &currInput );
	void Draw( sf::RenderTarget *target );
	void CompleteCurrentMap( SaveFile *sf, int totalFrames );
	Sector &GetCurrSector();
	int GetCurrSectorNumLevels();
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

	sf::Text currLevelTimeText;

	//ControllerState currInput;
	//ControllerState prevInput;
};

#endif