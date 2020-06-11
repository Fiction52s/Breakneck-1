 #ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

#include <SFML/Graphics.hpp>
#include <boost/filesystem.hpp>
#include "LevelSelector.h"
#include "Input.h"
#include "Tileset.h"
#include "ItemSelector.h"
#include "SaveFile.h"

struct Background;
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

struct Sector;
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
	bool IsFocused();
	void RunSelectedMap();
	void Init(Sector *sec);
	int GetNumLevels();
	void UpdateNodePosition();
	int GetSelectedIndex();
	Level &GetSelectedLevel();
	void Load();
	bool Update(ControllerState &curr,
		ControllerState &prev);
	void UpdateBG();
	void SetXCenter(float x);
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
	void UpdateLevelStats();
	void DrawLevelStats(sf::RenderTarget *target);
	void DrawStats(sf::RenderTarget *target);
	void DrawUnlockConditions(sf::RenderTarget *target);
	void UpdateUnlockConditions();
	void UpdateStats();

	State state;
	Sector *sec;
	MapSelector *ms;

	sf::Vector2f left;

	float xCenter;
	float percentComplete;

	int unlockedLevelCount;
	int frame;
	int stateFrame;
	int nodeSize;
	int pathLen;
	int numLevels;
	int unlockedIndex;
	int unlockFrame;
	int numTotalShards;
	int numUnlockConditions;
	int selectedYIndex;
	int sectorIndex;

	sf::Vertex *levelCollectedShards;
	sf::Vertex *levelCollectedShardsBG;
	sf::Vertex levelBG[4];
	sf::Vertex statsBG[4];
	sf::Vertex sectorStatsBG[4];
	sf::Sprite *nodes;
	sf::Sprite *topBonusNodes;
	sf::Sprite *botBonusNodes;
	sf::Sprite nodeExplodeSpr;
	sf::Sprite endSpr;

	sf::Text shardsCollectedText;
	sf::Text completionPercentText;
	sf::Text levelPercentCompleteText;
	sf::Text *unlockCondText;
	sf::Text sectorNameText;

	Tileset *ts_energyCircle;
	Tileset *ts_energyTri;
	Tileset *ts_energyMask;
	Tileset *ts_nodeExplode;
	Tileset *ts_shards;

	Background *bg;
};

struct MapSelector
{
	enum State
	{
		//S_SLIDINGLEFT,
		//S_SLIDINGRIGHT,
		S_SECTORSELECT,
		S_MAPSELECT,
	};

	enum KinState
	{
		K_STAND,
		K_JUMP,
		K_HIDE,
	};

	MapSelector(MainMenu *mm, sf::Vector2f &pos, 
		int wIndex);
	void UpdateHighlight();
	void RunSelectedMap();
	~MapSelector();
	MapSector *GetFocusedSector();
	void UpdateSprites();
	bool Update(ControllerState &curr,
		ControllerState &prev);
	void UpdateAllInfo(int index);
	void Draw(sf::RenderTarget *target);

	State state;
	MapSector **sectors;
	MainMenu *mainMenu;

	sf::Sprite newSelectTestSpr;
	sf::Sprite nodeHighlight;
	sf::Sprite kinSprite;
	sf::Sprite bottomBG;
	sf::Sprite thumbnailBG;
	sf::Sprite shardBG;

	KinState kinState;
	int worldIndex;
	int kinFrame;
	int frame;
	int numSectors;
	
	sf::Shader horizScrollShader1;
	sf::Shader horizScrollShader2;

	sf::Vertex backScrollEnergy[4];
	sf::Vertex frontScrollEnergy[4];
	
	sf::Vector2f sectorCenter;
	sf::Vector2f centerPos;
	MapNodeState *nodeStates[3];
	
	SingleAxisSelector *sectorSelector;
	SingleAxisSelector *mapSelector;

	Tileset *ts_sectorLevelBG;
	Tileset *ts_levelStatsBG;
	Tileset *ts_sectorStatsBG;
	Tileset *ts_scrollingEnergy;
	Tileset *ts_node;
	Tileset **ts_bossFight;
	Tileset *ts_sectorKey;
	Tileset **ts_sectorOpen;
	Tileset *ts_kinJump[5];
	
};

struct WorldSelector
{
	WorldSelector::WorldSelector(MainMenu *mm);
	void Update();
	void SetPosition(sf::Vector2f &pos);
	void SetAlpha(float alpha);
	void Draw(sf::RenderTarget *target);

	sf::Vector2f position;
	float angles[4];
	Tileset *ts;
	sf::Vertex quads[4 * 4];
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

	WorldMap(MainMenu *mainMenu);
	void RunSelectedMap();
	void Reset(SaveFile *sf);
	~WorldMap();
	void Update(
		ControllerState &prevInput,
		ControllerState &currInput);
	void Draw(sf::RenderTarget *target);
	void CompleteCurrentMap(SaveFile *sf, int totalFrames);
	Sector &GetCurrSector();
	int GetCurrSectorNumLevels();
	void UpdateMapList();
	const std::string & GetSelected();
	void UpdateMapList(TreeNode *parentNode, const std::string &relativePath);
	void ClearEntries();
	void UpdateColonySelect();
	int Tex(int index, int level, TreeNode *entry);
	void SetDefaultSelections();
	MapSelector *CurrSelector();
	void InitSelectors();

	State state;
	WorldSelector *worldSelector;
	MapSelector *selectors[7];
	MainMenu *mainMenu;
	
	sf::Shader zoomShader;
	sf::Shader asteroidShader;
	
	float currScale;
	float oldZoomCurvePos;
	float zoomCurvePos;

	sf::Sprite extraPassSpr;
	sf::Sprite selectorExtraPass;
	sf::Sprite colonySpr[7];
	sf::Sprite planetSpr;
	sf::Sprite spaceSpr;
	sf::Sprite colonySelectSpr;
	sf::Sprite colonySelectSprZoomed;
	
	sf::Vector2f currCenter;
	sf::Vector2f menuPos;
	
	sf::View zoomView;
	sf::View uiView;

	TreeNode **dirNode;
	TreeNode *entries;

	sf::Vertex asteroidQuads[4*4];
	sf::Vertex worldActiveQuads[7 * 4];
	sf::Vertex worldActiveQuadsZoomed[7 * 4];
	
	int frame;
	int asteroidFrame;
	int selectedLevel;
	int numTotalEntries;
	int selectedColony;
	int fontHeight;
	int leftBorder;
	int yspacing;
	
	Tileset *ts_space;
	Tileset *ts_planet;
	Tileset *ts_asteroids[4];
	Tileset *ts_colonyActiveZoomed[7];
	Tileset *ts_colonyActive[7];
	Tileset *ts_colony[7];
	Tileset *ts_colonySelect;
	
	sf::Text currLevelTimeText;
	sf::Text * text;

	sf::Font font;
	std::string * localPaths;
	
	sf::RectangleShape bgRect;
	sf::RectangleShape selectedRect;

	bool moveDown;
	bool moveUp;
};

#endif