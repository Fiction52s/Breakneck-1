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
struct WorldMapShip;
struct AdventureManager;
struct ParallelPracticeSettingsMenu;

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
		COMPLETE,
		LEADERBOARD,
		LEADERBOARD_STARTING,
	};


	State state;
	MapSelector *ms;
	Sector *sec;
	World *world;
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
	int sectorIndex;

	sf::Vertex levelCollectedShards[16 * 4];
	sf::Vertex levelStatsBG[4];
	sf::Vertex sectorStatsBG[4];
	sf::Vertex lockedOverlayQuad[4];
	sf::Vertex sectorArrowQuads[4 * 2];

	sf::Vertex levelSelectOptionQuads[4 * 4];
	sf::Vertex levelSelectOptionButtonQuads[5 * 4];

	sf::Vertex origPowersOptionQuad[4];
	sf::Vertex origPowersOptionButtonQuad[4];

	sf::Vertex levelNumberQuads[8 * 4];

	sf::Sprite *nodes;
	sf::Sprite nodeExplodeSpr;
	sf::Sprite endSpr;
	sf::Sprite mapPreviewSpr;
	sf::Sprite nodeHighlight;
	sf::Sprite mapShardIconSpr;
	sf::Sprite mapLogIconSpr;
	sf::Sprite mapBestTimeIconSpr;
	sf::Sprite sectorShardIconSpr;
	sf::Sprite sectorLogIconSpr;
	sf::Sprite lockSpr;

	sf::Text levelNameText;
	sf::Text bestTimeText;
	sf::Text shardsCollectedText;
	sf::Text logsCollectedText;
	sf::Text levelsBeatenText;
	//sf::Text requirementText;
	sf::Text sectorNameText;
	sf::Text numLevelsBeatenText;
	sf::Text sectorShardsCollectedText;
	sf::Text sectorLogsCollectedText;

	Tileset *ts_buttons;
	Tileset *ts_levelSelectNumbers;
	Tileset *ts_mapSelectOptions;
	Tileset *ts_origPowersOptions;

	Tileset *ts_energyTri;
	Tileset *ts_energyMask;
	Tileset *ts_nodeExplode;
	Tileset *ts_lock;
	//Tileset *ts_shards;
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
	void UpdateButtonIconsWhenControllerIsChanged();
	bool IsUnlocked();
	void UpdateUnlockedLevelCount();
	void DestroyBG();
	void CreateBG();
	void UpdateHighlight();
	//bool IsFocused();
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
	void DestroyMapPreview();
	void SetSelectedIndexToFurthestProgress();
	void UpdateSelectorSprite();
	bool Update(ControllerDualStateQueue *controllerInput );
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
	//void DrawRequirement(sf::RenderTarget *target);
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

	Tileset *ts_node;
	Tileset **ts_bossFight;
	Tileset **ts_sectorOpen;
	Tileset *ts_kinJump[5];
	Tileset *ts_rock;
	Tileset *ts_statIcons;

	//sf::Sprite bestTimeIconSpr;
	//sf::Sprite shardIconSpr;
	//sf::Sprite logIconSpr;
	

	MapSelector( WorldMap *worldMap, 
		World *world, MainMenu *mm, sf::Vector2f &pos );
	void ReturnFromMap();
	void RunSelectedMap();
	void Init();
	~MapSelector();
	MapSector *FocusedSector();
	void UpdateSprites();
	bool Update(ControllerDualStateQueue *controllerInput);
	void Draw(sf::RenderTarget *target);
	void CreateBGs();
	void DestroyBGs();
};

struct WorldSelector
{
	WorldSelector(MainMenu *mm);
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
		PLANET_VISUAL_ONLY,
		PLANET_PARALLEL_OPTIONS,
		PLANET,
		PlANET_TO_COLONY,
		COLONY,
		COLONY_TO_PLANET,
		PLANET_TO_SPACE,
		START_LEVEL
	};
	

	//int totalWorlds;

	ParallelPracticeSettingsMenu *parallelPracticeSettings;

	State state;
	WorldSelector *worldSelector;
	MapSelector **selectors;
	MainMenu *mainMenu;

	sf::Shader zoomShader;
	sf::Shader asteroidShader;

	float currScale;
	float oldZoomCurvePos;
	float zoomCurvePos;

	sf::Shader selectableRingShader;

	sf::Sprite extraPassSpr;
	sf::Sprite selectorExtraPass;
	sf::Sprite colonySpr[ADVENTURE_MAX_NUM_WORLDS];
	sf::Sprite planetSpr;
	sf::Sprite spaceSpr;
	sf::Sprite colonySelectSpr;
	sf::Sprite colonySelectSprZoomed;
	sf::Sprite parallelPlayMarkerSpr;
	sf::Sprite parallelPlaySettingsMarkerSpr;

	float colonyRadius;

	sf::Vector2f currCenter;
	sf::Vector2f menuPos;

	sf::View zoomView;
	sf::View uiView;

	TreeNode **dirNode;
	TreeNode *entries;

	
	sf::Vertex parallelPlayButtonQuad[4 * 1];
	sf::Vertex parallelPlaySettingsButtonQuad[4 * 1];

	sf::Vertex infoNameBG[4];
	sf::Vertex infoQuadBG[4];
	sf::Text sectorsCompleteText;
	sf::Text shardsCapturedText;
	sf::Text logsCapturedText;
	sf::Text worldNameText;

	sf::Text numCurrentPlayersText[ADVENTURE_MAX_NUM_WORLDS];

	sf::Vertex asteroidQuads[4 * 4];
	sf::Vertex worldActiveQuads[ADVENTURE_MAX_NUM_WORLDS * 4];
	sf::Vertex worldActiveQuadsZoomed[ADVENTURE_MAX_NUM_WORLDS * 4];
	sf::Vertex worldSelectableQuads[ADVENTURE_MAX_NUM_WORLDS * 4];

	int frame;
	int asteroidFrame;
	int selectedLevel;
	int numTotalEntries;
	int selectedColony;
	int fontHeight;
	int leftBorder;
	int yspacing;

	Tileset *ts_buttons;
	Tileset *ts_space;
	Tileset *ts_planet;
	Tileset *ts_asteroids[4];
	Tileset * ts_colonyActiveZoomed[ADVENTURE_MAX_NUM_WORLDS];
	Tileset * ts_colonyActive[ADVENTURE_MAX_NUM_WORLDS];
	Tileset * ts_colony[ADVENTURE_MAX_NUM_WORLDS];
	Tileset *ts_selectableRing;
	Tileset *ts_parallelPlayMarker;
	Tileset *ts_parallelPlaySettingsMarker;

	AdventureManager *adventureManager;

	sf::Text currLevelTimeText;
	sf::Text * text;

	sf::Font &font;
	std::string * localPaths;

	sf::RectangleShape bgRect;
	sf::RectangleShape selectedRect;

	bool moveDown;
	bool moveUp;
	bool allUnlocked;

	WorldMapShip *ship;

	WorldMap();
	void RunSelectedMap();
	void Reset(SaveFile *sf);
	~WorldMap();
	void Update();
	void Draw(sf::RenderTarget *target);
	Sector &GetCurrSector();
	int GetCurrSectorNumLevels();
	const std::string & GetSelected();
	void ClearEntries();
	void UpdateColonySelect();
	void UpdateWorldStats();
	void SetDefaultSelections();
	MapSelector *CurrSelector();
	void InitSelectors();
	bool IsInAllUnlockedMode();
	void UpdateSelectedColony();
	void SetShipToColony(int index);
	sf::Vector2f GetColonyCenter(int index);
	void SetToColony(int selColony);
	void SetToLevel(int selColony, int sec, int m);
	void HandleEvent(sf::Event ev );
	void UpdateButtonIconsWhenControllerIsChanged();
private:
	void SetupAsteroids();
	void LoadAdventure(const std::string &adventureName);
	void DrawAsteroids(sf::RenderTarget *target, bool back);
};

#endif