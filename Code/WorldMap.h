 #ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

#include <SFML/Graphics.hpp>
#include <boost/filesystem.hpp>
#include "LevelSelector.h"
#include "Input.h"
#include "Tileset.h"
#include "ItemSelector.h"
#include "SaveFile.h"
#include <boost/thread.hpp>
#include "PlayerSkinShader.h"

struct Background;
struct MainMenu;
struct SaveFile;
struct MapSelector;
struct MapSector;
struct WorldMap;
struct RecordGhostMenu;

enum MapNodeState
{
	NS_DEACTIVATED,
	NS_ACTIVATED,
	NS_PARTIALLY_BEATEN,
	NS_FULLY_BEATEN
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


	State state;
	MapSelector *ms;
	Sector *sec;
	SaveFile *saveFile;
	AdventureFile &adventureFile;
	float mapPreviewHeight;

	bool bestTimeGhostOn;
	bool bestReplayOn;

	bool ghostAndReplayOn;

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
	int numRequiredRunes;
	int sectorIndex;

	sf::Vertex levelCollectedShards[16 * 4];
	sf::Vertex levelBG[4];
	sf::Vertex levelStatsBG[4];
	sf::Vertex sectorStatsBG[4];
	sf::Vertex lockedOverlayQuad[4];
	sf::Vertex sectorArrowQuads[4 * 2];

	sf::Vertex levelSelectOptionQuads[4 * 3];
	sf::Vertex levelSelectOptionButtonQuads[4 * 3];

	sf::Vertex levelNumberQuads[8 * 4];

	sf::Sprite *nodes;
	sf::Sprite nodeExplodeSpr;
	sf::Sprite endSpr;
	sf::Sprite mapPreviewSpr;
	sf::Sprite nodeHighlight;
	sf::Sprite mapShardIconSpr;
	sf::Sprite mapBestTimeIconSpr;
	sf::Sprite sectorShardIconSpr;

	sf::Text bestTimeText;
	sf::Text shardsCollectedText;
	sf::Text levelsBeatenText;
	sf::Text requirementText;
	sf::Text sectorNameText;
	sf::Text numLevelsBeatenText;
	sf::Text sectorShardsCollectedText;

	Tileset *ts_mapOptionButtons;
	Tileset *ts_levelSelectNumbers;
	Tileset *ts_mapSelectOptions;
	Tileset *ts_energyCircle;
	Tileset *ts_energyTri;
	Tileset *ts_energyMask;
	Tileset *ts_nodeExplode;
	Tileset *ts_shards;
	Tileset *ts_sectorArrows;

	Tileset *ts_mapPreview;

	Tileset *ts_menuSelector;
	sf::Sprite selectorSprite;
	int selectorAnimFrame;
	int selectorAnimDuration;
	int selectorAnimFactor;

	int sectorArrowFrame;
	
	SingleAxisSelector *mapSASelector;
	std::string bgName;
	Background *bg;

	MapSector( AdventureFile &adventureFile,
		Sector *p_sector, 
		MapSelector *ms, int index);
	~MapSector();
	void UpdateUnlockedLevelCount();
	void DestroyBG();
	void CreateBG();
	void UpdateHighlight();
	bool IsFocused();
	void RunSelectedMap();
	void Init(SaveFile *sf);
	int GetNumLevels();
	void UpdateNodePosition();
	int GetSelectedIndex();
	Level *GetSelectedLevel();
	void UpdateOptionButtons();
	void UpdateSectorArrows();
	AdventureMap *GetSelectedAdventureMap();
	void Load();
	void UpdateMapPreview();
	void UpdateSelectorSprite();
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
	void UpdateLevelStats();
	void DrawLevelStats(sf::RenderTarget *target);
	void DrawStats(sf::RenderTarget *target);
	void DrawRequirement(sf::RenderTarget *target);
	void UpdateStats();
};

struct MapSelector
{
	enum State
	{
		//S_SLIDINGLEFT,
		//S_SLIDINGRIGHT,
		S_SECTORSELECT,
		S_MAPSELECT,
		S_CHANGINGSECTORS
	};

	enum KinState
	{
		K_STAND,
		K_JUMP,
		K_HIDE,
	};


	PlayerSkinShader playerSkinShader;
	State state;
	std::vector<MapSector*> sectors;
	MainMenu *mainMenu;
	WorldMap *worldMap;

	//RecordGhostMenu *recordGhostMenu;
	
	sf::Sprite kinSprite;
	sf::Sprite bottomBG;
	sf::Sprite thumbnailBG;
	sf::Sprite shardBG;
	sf::Sprite rockSprite;

	KinState kinState;
	int kinFrame;
	int frame;
	int numSectors;

	sf::Vector2f sectorCenter;
	sf::Vector2f centerPos;
	MapNodeState *nodeStates[3];

	SingleAxisSelector *sectorSASelector;
	World *world;

	Tileset *ts_sectorLevelBG;
	Tileset *ts_levelStatsBG;
	Tileset *ts_sectorStatsBG;
	Tileset *ts_node;
	Tileset **ts_bossFight;
	Tileset *ts_sectorKey;
	Tileset **ts_sectorOpen;
	Tileset *ts_kinJump[5];
	Tileset *ts_rock;
	Tileset *ts_statIcons;

	sf::Sprite bestTimeIconSpr;
	sf::Sprite shardIconSpr;
	

	MapSelector( WorldMap *worldMap, 
		World *world, MainMenu *mm, sf::Vector2f &pos );
	void ReturnFromMap();
	void RunSelectedMap();
	void Init();
	~MapSelector();
	MapSector *FocusedSector();
	void UpdateSprites();
	bool Update(ControllerState &curr,
		ControllerState &prev);
	void Draw(sf::RenderTarget *target);
	void CreateBGs();
	void DestroyBGs();
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

struct KinBoostScreen;
struct WorldMap : TilesetManager
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

	//moved this here
	//because its only needed in adventure 
	//currently.
	KinBoostScreen *kinBoostScreen;

	const static int MAX_NUM_WORLDS = 8;

	State state;
	WorldSelector *worldSelector;
	MapSelector **selectors;
	MainMenu *mainMenu;

	AdventureFile adventureFile;
	Planet *planet;

	sf::Shader zoomShader;
	sf::Shader asteroidShader;

	float currScale;
	float oldZoomCurvePos;
	float zoomCurvePos;

	sf::Sprite extraPassSpr;
	sf::Sprite selectorExtraPass;
	sf::Sprite colonySpr[MAX_NUM_WORLDS];
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

	sf::Vertex infoNameBG[4];
	sf::Vertex infoQuadBG[4];
	sf::Text sectorsCompleteText;
	sf::Text shardsCapturedText;
	sf::Text worldNameText;

	sf::Vertex asteroidQuads[4 * 4];
	sf::Vertex worldActiveQuads[MAX_NUM_WORLDS * 4];
	sf::Vertex worldActiveQuadsZoomed[MAX_NUM_WORLDS * 4];

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
	Tileset *ts_colonyActiveZoomed[MAX_NUM_WORLDS];
	Tileset *ts_colonyActive[MAX_NUM_WORLDS];
	Tileset *ts_colony[MAX_NUM_WORLDS];
	Tileset *ts_colonySelect;

	sf::Text currLevelTimeText;
	sf::Text * text;

	sf::Font font;
	std::string * localPaths;

	sf::RectangleShape bgRect;
	sf::RectangleShape selectedRect;

	bool moveDown;
	bool moveUp;
	bool allUnlocked;

	WorldMap(MainMenu *mainMenu);
	void RunSelectedMap();
	void Reset(SaveFile *sf);
	~WorldMap();
	void Update(
		ControllerState &prevInput,
		ControllerState &currInput);
	void Draw(sf::RenderTarget *target);
	void CompleteCurrentMap( Level *level, int totalFrames);
	Sector &GetCurrSector();
	int GetCurrSectorNumLevels();
	void UpdateMapList();
	const std::string & GetSelected();
	void UpdateMapList(TreeNode *parentNode, const std::string &relativePath);
	void ClearEntries();
	void UpdateColonySelect();
	void UpdateWorldStats();
	int Tex(int index, int level, TreeNode *entry);
	void SetDefaultSelections();
	MapSelector *CurrSelector();
	void InitSelectors();
	bool IsInAllUnlockedMode();
};

#endif