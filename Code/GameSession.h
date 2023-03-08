#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

#include "Physics.h"
#include "Tileset.h"
#include <list>
#include "QuadTree.h"
#include <SFML/Graphics.hpp>
#include "Light.h"
#include "Camera.h"
#include "Gate.h"
#include "Zone.h"
#include "AirParticles.h"
#include "Movement.h"
#include "SoundManager.h"
#include <map>
#include "BarrierReactions.h"
#include "Buf.h"
#include "EnvEffects.h"
#include "EffectLayer.h"
#include <boost/filesystem.hpp>"
#include "DecorTypes.h"
#include <boost/thread/mutex.hpp>
#include "earcut.hpp"
#include "PoiInfo.h"

#include "Session.h"
#include "GamePopup.h"


struct BonusHandler;

struct ZoneNode;

struct Actor;
struct ComboObject;

struct MatchResultsScreen;
struct BoxEmitter;
struct ShapeEmitter;
struct Minimap;
struct ScreenRecorder;
struct TopClouds;
struct TerrainRender;
struct ReplayPlayer;
struct ReplayGhost;
struct Config;
struct InputVisualizer;
struct TerrainDecorInfo;

struct BasicBossScene;


struct ImageText;
struct TimerText;

struct AbsorbParticles;
struct Tileset;

struct Fader;
struct Swiper;

struct Barrier;

struct BitField;

struct ShardPopup;


struct GameSession;
struct KeyMarker;

struct PowerOrbs;
struct FillRing;
struct DesperationOrb;

struct Grass;


struct SaveFile;
struct Level;
struct MainMenu;

struct GoalPulse;
struct PauseMenu;
struct Sequence;
struct EnterNexus1Seq;
struct ShipExitScene;

struct UIWindow;
struct Parallax;



struct MusicInfo;
struct GhostEntry;
struct ResultsScreen;
struct RaceFightTarget;
struct BasicEffect;
struct StorySequence;
struct AirTrigger;
struct Nexus;
struct ButtonHolder;

struct DecorExpression;
struct DecorLayer;
struct TouchGrassCollection;

//struct HealthFly;

struct EnvPlant;

struct Barrier;

namespace mapbox
{
	namespace util
	{
		template<>
		struct nth<0, sf::Vector2i> {
			inline static auto get(const sf::Vector2i &t) {
				return t.x;
			}
		};

		template<>
		struct nth<1, sf::Vector2i> {
			inline static auto get(const sf::Vector2i &t)
			{
				return t.y;
			}
		};
	}
}



enum EdgeAngleType
{
	EDGE_FLAT,
	EDGE_SLOPED,
	EDGE_STEEPSLOPE,
	EDGE_WALL,
	EDGE_STEEPCEILING,
	EDGE_SLOPEDCEILING,
	EDGE_FLATCEILING
};

EdgeAngleType GetEdgeAngleType(V2d &normal);

struct ZonePropertiesObj
{
	ZonePropertiesObj(
		sf::Vector2i &p_pos,
		int p_zoneType,
		float p_drainFactor)
		:pos(p_pos),
		zoneType(p_zoneType),
		drainFactor(p_drainFactor)
	{

	}

	sf::Vector2i pos;
	int zoneType;
	float drainFactor;
};


struct GameSession : RayCastHandler, Session
{
	//maybe move this out eventually
	struct DecorInfo
	{
		DecorInfo(sf::Sprite &sp,
			int lay, Tileset *t, int p_tile)
			:spr(sp), layer(lay), ts(t), tile(p_tile)
		{

		}
		sf::Sprite spr;
		int layer;
		Tileset *ts;
		int tile;
	};

	struct DecorDraw
	{
		DecorDraw(sf::Vertex *q,
			int numVerts,
			Tileset *t);
		~DecorDraw();
		void Draw(sf::RenderTarget *target);
		sf::Vertex *quads;
		Tileset *ts;
		int numVertices;
	};

	enum GameResultType
	{
		GR_BONUS_RETURN,
		GR_WIN,
		GR_WINCONTINUE,
		GR_EXITLEVEL,
		GR_EXITTITLE,
		GR_EXITGAME,
		GR_BONUS_RESPAWN,
	};

	enum PopupType
	{
		POPUPTYPE_NO_REPLAY_FOUND,
		POPUPTYPE_NO_GHOST_FOUND,
		POPUPTYPE_Count,
	};


	enum SoundType
	{
		S_KEY_COMPLETE_W1,
		S_KEY_COMPLETE_W2,
		S_KEY_COMPLETE_W3,
		S_KEY_COMPLETE_W4,
		S_KEY_COMPLETE_W5,
		S_KEY_COMPLETE_W6,
		S_KEY_ENTER_0,
		S_KEY_ENTER_1,
		S_KEY_ENTER_2,
		S_KEY_ENTER_3,
		S_KEY_ENTER_4,
		S_KEY_ENTER_5,
		S_KEY_ENTER_6,
		Count
	};

	enum EdgeType
	{
		FLAT_GROUND,
		SLOPED_GROUND,
		STEEP_GROUND,
		WALL,
		STEEP_CEILING,
		SLOPED_CEILING,
		CEILING
	};

	static GameSession *GetSession();
	static GameSession *currSession;


	std::string myHash;
	int gameStatePrePopup;
	int currPopupType;
	GamePopup *gamePopup;

	//new
	bool boostEntrance;
	bool bestTimeGhostOn;
	bool bestReplayOn;



	//from mainmenu

	//for loading only
	std::list<PolyPtr> allPolygonsList;
	bool continueLoading;
	boost::mutex continueLoadingLock;
	std::set<std::pair<int, int>> matSet;
	//std::map<int, int> matIndices;
	std::map<std::string, std::list<DecorInfo>> decorListMap;

	ScreenRecorder *debugScreenRecorder;
	SaveFile *saveFile;
	Config *config;

	sf::Text replayText;
	
	PauseMenu *pauseMenu;

	//does not change during running
	std::list<PolyPtr> allSpecialTerrain;
	
	bool usePolyShader;
	bool hasGrass[6];
	bool hasAnyGrass;
	
	SoundInfo * gameSoundInfos[SoundType::Count];
	
	
	std::vector<RailPtr> allRails;
	int totalRails;
	
	std::list<boost::filesystem::path> bonusPaths;

	//for queries

	//raycast temps
	
	
	ShapeEmitter *testEmit;
	std::list<DecorDraw*> decor[EffectLayer::EFFECTLAYER_Count];
	
	int numKeysCollected;

	
	
	
	V2d nexusPos;
	Nexus *nexus;
	
	std::vector<ReplayGhost*> replayGhosts;
	Grass *explodingGravityGrass;
	bool quit;
	bool boostIntro;
	
	Level *level;
	
	EnvPlant *activeEnvPlants;
	
	
	int m_numActivePlayers;
	sf::Shader timeSlowShader; //actually time slow shader
	
	GameSession *bonusGame; //make this a container later
	
	bool activateBonus;
	V2d bonusReturnPos;
	BonusHandler *bonusHandler;
	int bonusType;
	
	Gate *unlockedGateList;
	sf::Vector2f oldCamBotLeft;
	sf::View oldView;
	
	sf::VertexArray *va;
	std::list<sf::VertexArray*> polygonBorders;

	
	bool shadersLoaded;//to prevent reloading of shaders
	
	std::map<DecorType, DecorLayer*> decorLayerMap;
	std::list<DecorLayer*> DecorLayers;
	
	
	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;
	
	GameResultType resType;
	sf::Sprite kinMapSpawnIcon;
	
	
	QuadTree *terrainBGTree;
	QuadTree * itemTree;
	QuadTree *inverseEdgeTree;
	QuadTree *activeEnemyItemTree;
	QuadTree *envPlantTree;
	std::list<EnvPlant*> allEnvPlants;
	QuadTree *airTriggerTree;
	std::list<AirTrigger*> fullAirTriggerList;
	bool showTerrainDecor;
	
	bool isDefaultKeyboardInputOn;
	
	
	int returnVal;

	


	std::string GetBestTimeGhostPath();
	std::string GetBestReplayPath();
	GameSession * CreateBonus(const std::string &bonusName,
		int p_bonusType = BONUSTYPE_DEFAULT);

	GameSession *CreateParallelSession( int parIndex );

	void SetBonus(GameSession *bonus,
		V2d &returnPos, 
		BonusHandler *bHandler = NULL);
	void ReturnFromBonus();
	static int IsFlatGround(sf::Vector2<double> &normal);
	static int IsSlopedGround(sf::Vector2<double> &normal);
	static int IsSteepGround(sf::Vector2<double> &normal);
	static int IsWall(sf::Vector2<double> &normal);

	GameSession(MatchParams *mp);//SaveFile *sf,
		//const boost::filesystem::path &p_filePath);
	~GameSession();

	

	
	//process reading file
	void ProcessHeader();
	void ProcessDecorSpr(const std::string &name,
		Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
		float rotation, sf::Vector2f &scale);
	void ProcessAllDecorSpr();
	void ProcessTerrain(PolyPtr poly);
	void ProcessAllTerrain();
	void ProcessActor(ActorPtr a);
	void ProcessAllActors();
	void ProcessGate(int gCat, 
		int gVar, int numToOpen,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex, int seconds);
	void ProcessRail(RailPtr rail);
	void ProcessSpecialTerrain(PolyPtr poly);

	//gamesession itself
	int Run();
	void Reload(const boost::filesystem::path &p_filePath);
	static bool sLoad(GameSession *gs);
	bool Load();
	void HandleRayCollision(Edge *edge,
		double edgeQuantity, double rayPortion);
	void SetContinueLoading(bool cont);
	bool ShouldContinueLoading();
	void Init();
	void Cleanup();
	void CheckSinglePlayerInputDefaultKeyboard();

	//fader
	
	
	void SetBestReplayOn(bool on);
	void SetBestGhostOn(bool on);

	bool SetupMyBestPlayerReplayManager();
	void CleanupReplaysAndGhosts();
	PolyPtr GetPolygon(int index);
	
	void UpdateDecorSprites();
	
	
	void ActivateAbsorbParticles(int absorbType, Actor *p, int storedHits,
		V2d &pos, float startAngle = 0);
	
	void UpdatePolyShaders(sf::Vector2f &botLeft,
		sf::Vector2f &playertest);
	
	void RestartGame();
	void RestartLevel();
	void NextFrameRestartLevel();
	
	//setup
	void SetupGhosts(std::list<GhostEntry*> &ghosts);
	void SetupPlayers();
	void SetupShaders();
	void SetupBackground();
	
	void SetupQuadTrees();
	bool SetupControlProfiles();
	
	void SetupPlayerRecordingManager();
	
	sf::VertexArray *SetupBushes(int bgLayer,
		Edge *startEdge, Tileset *ts);
	sf::VertexArray * SetupPlants(Edge *start, Tileset *ts);

	//player
	int GetPlayerTeamIndex(int index = 0);
	
	int GetPlayerHitstunFrames(int index = 0);
	

	//draw

	void DebugDraw(sf::RenderTarget *target);
	void DrawActiveEnvPlants();


	
	void QuitGame();
	void DrawRails(sf::RenderTarget *target);
	void DrawReplayGhosts(sf::RenderTarget *target);
	void UpdateDebugModifiers();
	void SetStorySeq(StorySequence *storySeq);
	void SuppressEnemyKeys(Gate *g);
	bool IsShardCaptured(int sType);
	void SetOriginalMusic();
	void UpdateTimeSlowShader();
	void UpdateEnvShaders();
	void UnlockUpgrade(int upgradeType, int playerIndex = 0 );
	bool TrySaveCurrentSaveFile();
	void UnlockLog(int lType, int playerIndex = 0);
	void UpdateExplodingGravityGrass();
	void AddGravityGrassToExplodeList(Grass *g);
	void RemoveGravityGrassFromExplodeList(Grass *g);

	void RecordReplayEnemies();
	void UpdateEnemiesSprites();
	
	void ResetPlants();
	void ResetInactiveEnemies();
	void rResetPlants( QNode *node );
	int CountActiveEnemies();
	SaveFile *GetCurrSaveFile();
	
	void SoftenGates(int gCat);
	void ReformGates(int gCat );
	void OpenGates(int gCat);

	void CloseGates(int gCat);
	
	int TryToActivateBonus();
	void UpdateReplayGhostSprites();
	
	
	void DrawSpecialTerrain(sf::RenderTarget *target);
	void DrawFlyTerrain(sf::RenderTarget *target);
	void DrawTerrain(sf::RenderTarget *target);
	void DrawSceneToPostProcess(sf::RenderTexture *tex);
	void DrawShockwaves(sf::RenderTarget *target);
	void UpdateCamera();
	void UpdateEnvPlants();
	bool UpdateRunModeBackAndStartButtons();
	void CleanupDecor();
	void CleanupGhosts();
	void DrawDecor(EffectLayer ef, sf::RenderTarget *target);

	bool RunPreUpdate();
	void UpdateSoundNodeLists();

	void RecGhostRecordFrame();
	bool RunPostUpdate();
	bool PopupGameModeUpdate();
	bool LeaderboardGameModeUpdate();
	void SequenceGameModeRespondToGoalDestroyed();
	void UpdateTerrainStates();
	void UpdateRailStates();

	void SetupPauseMenu();
	void CleanupPauseMenu();

	void SetupPopup();
	void CleanupPopup();

	void UpdatePopup();
	void OpenPopup(int popType);

	bool HasLog(int logIndex);
	int GetBonusType();
	//int GetPlayerNormalSkin(int index);

	bool RunMainLoopOnce(); //return false means go again
	MatchResultsScreen *CreateResultsScreen();

	void DrawLeaderboard(sf::RenderTarget *target);
	void StartLeaderboard();

	bool TryStartMyBestReplay();
	bool TryStartLeaderboardReplay( PlayerReplayManager *prm );
	bool AddGhosts();
	bool AddGhostsForReplay(PlayerReplayManager *prm);
	bool TryStartGhosts();
	void RestartWithNoReplayOrGhosts();
};
#endif