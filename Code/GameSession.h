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


#include "Session.h"

struct ZoneNode;
struct GateMarkerGroup;

struct Actor;
struct ComboObject;

struct MapHeader;

struct BoxEmitter;
struct ShapeEmitter;
struct Minimap;
struct ScreenRecorder;
struct TopClouds;
struct TerrainRender;
struct RecordPlayer;
struct ReplayPlayer;
struct RecordGhost;
struct ReplayGhost;
struct Config;
struct RaceFightHUD;
struct InputVisualizer;
struct MomentaBroadcast;
struct TerrainDecorInfo;

struct BasicBossScene;

struct LoadingMapProgressDisplay;


struct ImageText;
struct TimerText;

struct AbsorbParticles;
struct Tileset;

struct AdventureHUD;
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


struct DialogueUser;
struct GoalPulse;
struct PauseMenu;
struct Sequence;
struct EnterNexus1Seq;
struct ShipExitScene;

struct VictoryScreen;
struct VictoryScreen2PlayerVS;
struct UIWindow;
struct Parallax;
struct ScoreDisplay;


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

struct PoiInfo
{
	PoiInfo( const std::string &name, sf::Vector2i &p );
	PoiInfo( const std::string &name, Edge *e, double q );
	sf::Vector2<double> pos;	
	Edge *edge;
	double edgeQuantity;
	std::string name;
};



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

struct KeyNumberObj
{
	KeyNumberObj(sf::Vector2i &p_pos,
		int nKeys, int zt)
		:pos(p_pos), numKeys(nKeys),
		zoneType(zt)
	{
	}
	sf::Vector2i pos;
	int numKeys;
	int zoneType;
};

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

struct GameSession : QuadTreeCollider, RayCastHandler, Session
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

	struct RaceFight
	{
		RaceFight(GameSession *owner,
			int raceFightMaxSeconds);
		void Reset();
		int playerScore;
		int player2Score;
		void DrawScore(sf::RenderTarget *target);
		void UpdateScore();
		void Init();

		RaceFightHUD *hud;
		//RaceFightTarget *targetList;
		GameSession *owner;

		sf::Text tempAllTargets;

		ResultsScreen *victoryScreen;

		RaceFightTarget *hitByPlayerList;
		RaceFightTarget *hitByPlayer2List;
		void HitByPlayer(int playerIndex,
			RaceFightTarget *target);

		void PlayerHitByPlayer(int attacker,
			int defender);
		void TickClock();
		void TickFrame();
		int NumDigits(int number);
		void RemoveFromPlayerHitList(RaceFightTarget *target);
		void RemoveFromPlayer2HitList(RaceFightTarget *target);
		int frameCounter;

		int numTargets;
		ImageText *playerScoreImage;
		ImageText *player2ScoreImage;
		int raceFightResultsFrame;
		TimerText *gameTimer;
		ImageText *numberTargetsRemainingImage;
		ImageText *numberTargetsTotalImage;
		int GetNumRemainingTargets();

		UIWindow *testWindow;

		int playerHitCounter;
		int player2HitCounter;

		bool gameOver;
		int place[4];
		//int p3Place;
		//int p4Place;
		int raceWinnerIndex;
	};

	enum GameResultType
	{
		GR_WIN,
		GR_WINCONTINUE,
		GR_EXITLEVEL,
		GR_EXITTITLE,
		GR_EXITGAME
	};

	enum State
	{
		RUN,
		CUTPAUSE,
		CUTSCENE,
		PAUSE,
		RACEFIGHT_RESULTS,
		STORY,
		SEQUENCE,
		FROZEN,
		MAP
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

	
	std::list<PolyPtr> allSpecialTerrain;
	PolyPtr specialPieceList;
	PolyPtr flyTerrainList;
	PolyPtr inversePoly;
	std::list<PolyPtr> allPolygonsList;
	std::map<std::string, std::list<DecorInfo>> decorListMap;
	sf::View view; //need to be added to session
	std::set<std::pair<int, int>> matSet;
	bool usePolyShader;
	bool hasGrass[6];
	bool hasAnyGrass;
	PolyPtr polyQueryList;
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;
	Edge *rcEdge;
	double rcQuantity;
	Edge *rayIgnoreEdge;
	Edge *rayIgnoreEdge1;
	Fader *fader;
	Swiper *swiper;
	sf::Vertex blackBorderQuads[4 * 4];
	AbsorbParticles *absorbParticles;
	AbsorbParticles *absorbDarkParticles;
	AbsorbParticles *absorbShardParticles;
	bool continueLoading;
	boost::mutex continueLoadingLock;
	ShapeEmitter *testEmit;
	std::list<DecorDraw*> decorBetween;
	Minimap *mini;
	PauseMenu *pauseMenu;
	SaveFile *saveFile;
	Config *config;
	ScoreDisplay *scoreDisplay;
	AdventureHUD *adventureHUD;
	Sequence *activeSequence;
	BasicBossScene *preLevelScene;
	BasicBossScene *postLevelScene;
	StorySequence *currStorySequence;
	MomentaBroadcast *currBroadcast;
	DialogueUser *activeDialogue;
	TopClouds *topClouds;
	bool stormCeilingOn;
	double stormCeilingHeight;
	ScreenRecorder *debugScreenRecorder;
	std::list<KeyNumberObj*> keyNumberObjects;
	int numKeysCollected;
	ShardPopup *shardPop;
	BitField *shardsCapturedField;
	Sequence *getShardSeq;
	Sequence *deathSeq;
	RaceFight *raceFight;
	MusicInfo *originalMusic;
	std::map<std::string, MusicInfo*> musicMap;
	sf::SoundBuffer * gameSoundBuffers[SoundType::Count];
	sf::VertexArray * goalEnergyFlowVA;
	GoalPulse *goalPulse;
	bool hasGoal;
	sf::Vector2<double> goalPos;
	sf::Vector2<double> goalNodePos;
	sf::Vector2<double> goalNodePosFinal;
	V2d nexusPos;
	Nexus *nexus;
	sf::Shader flowShader;
	float flowRadius;
	int flowFrameCount;
	int flowFrame;
	float maxFlowRadius;
	float radDiff;
	float flowSpacing;
	float maxFlowRings;
	std::list<RailPtr> allRails;
	RailPtr railDrawList;
	int totalRails;
	Buf testBuf;//for recording ghost
	RecordGhost *recGhost;
	RecordPlayer *recPlayer;
	ReplayPlayer *repPlayer;
	std::list<ReplayGhost*> replayGhosts;
	Tileset *ts_gravityGrass;
	Grass *explodingGravityGrass;
	//ship sequence. should be in its own structure
	Tileset *ts_w1ShipClouds0;
	Tileset *ts_w1ShipClouds1;
	Tileset *ts_ship;
	sf::VertexArray cloud0;
	sf::VertexArray cloud1;
	sf::VertexArray cloudBot0;
	sf::VertexArray cloudBot1;
	sf::Vector2f relShipVel;
	sf::Sprite shipSprite;
	sf::RectangleShape middleClouds;
	bool shipSequence;
	bool hasShipEntrance;
	V2d shipEntrancePos;
	sf::Vector2f cloudVel;
	int shipSeqFrame;
	sf::Vector2f shipStartPos;
	ShipExitScene *shipExitScene;
	bool drain;
	State state;
	sf::Font font;
	bool quit;
	bool boostIntro;
	InputVisualizer *inputVis;
	Level *level;
	bool playerAndEnemiesFrozen;
	EnvPlant *activeEnvPlants;
	int totalFramesBeforeGoal;
	std::map<std::string, PoiInfo*> poiMap;
	std::map<std::string, CameraShot*> cameraShotMap;
	std::map<std::string, Barrier*> barrierMap;
	std::list<Barrier*> barriers;
	int m_numActivePlayers;
	sf::Shader cloneShader; //actually time slow shader
	std::list<Edge*> globalBorderEdges;
	std::list<boost::filesystem::path> bonusPaths;
	GameSession *bonusGame; //make this a container later
	int testGateCount;
	Gate *gateList;
	Gate *unlockedGateList;
	float oldZoom;
	sf::Vector2f oldCamBotLeft;
	sf::View oldView;
	sf::Shader glowShader;
	sf::Shader motionBlurShader;
	sf::Shader hBlurShader;
	sf::Shader vBlurShader;
	sf::Shader shockwaveShader;
	sf::Vector2f testShock;
	sf::Texture shockwaveTex;
	int shockTestFrame;
	LoadingMapProgressDisplay *progressDisplay;
	sf::VertexArray *va;
	Collider coll;
	std::list<sf::VertexArray*> polygonBorders;
	sf::RenderTexture *lastFrameTex;
	sf::RenderTexture *preScreenTex;
	sf::RenderTexture *postProcessTex;
	sf::RenderTexture *postProcessTex1;
	sf::RenderTexture *postProcessTex2;
	sf::RenderTexture *minimapTex;
	sf::RenderTexture *mapTex;
	sf::RenderTexture *pauseTex;
	Rain *rain;
	const static int NUM_CLOUDS = 5;
	sf::Sprite clouds[NUM_CLOUDS];
	Tileset *cloudTileset;
	bool shadersLoaded;
	std::string rayMode;
	std::map<DecorType, DecorLayer*> decorLayerMap;
	std::list<DecorLayer*> DecorLayers;
	std::string queryMode;
	bool drawInversePoly;
	Edge *inverseEdgeList;
	int numBorders;
	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;
	bool goalDestroyed;
	GameResultType resType;
	sf::View cloudView;
	sf::Sprite kinMapSpawnIcon;
	Tileset *ts_mapIcons;
	ShapeEmitter *emitterLists[EffectLayer::Count];
	sf::Rect<double> tempSpawnRect;
	QuadTree *terrainBGTree;
	QuadTree * enemyTree;
	QuadTree * gateTree;
	QuadTree * itemTree;
	QuadTree *specterTree;
	QuadTree *inverseEdgeTree;
	QuadTree *railDrawTree;
	QuadTree *activeEnemyItemTree;
	QuadTree *envPlantTree;
	std::list<EnvPlant*> allEnvPlants;
	QuadTree *airTriggerTree;
	std::list<AirTrigger*> fullAirTriggerList;
	bool nextFrameRestart;
	bool showTerrainDecor;

	static int IsFlatGround(sf::Vector2<double> &normal);
	static int IsSlopedGround(sf::Vector2<double> &normal);
	static int IsSteepGround(sf::Vector2<double> &normal);
	static int IsWall(sf::Vector2<double> &normal);

	GameSession(SaveFile *sf,
		const boost::filesystem::path &p_filePath);
	~GameSession();

	//process reading file
	void ProcessHeader();
	void ProcessDecorSpr(const std::string &name,
		Tileset *d_ts, int dTile, int dLayer, sf::Vector2f &centerPos,
		float rotation, sf::Vector2f &scale);
	void ProcessAllDecorSpr();
	void ProcessPlayerStartPos();
	void ProcessTerrain(PolyPtr poly);
	void ProcessAllTerrain();
	void ProcessActor(ActorPtr a);
	void ProcessAllActors();
	void ProcessGate(int gCat, 
		int gVar, int numToOpen,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex);
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

	//fader
	void Fade(bool in, int frames, sf::Color c, bool skipKin = false);
	void CrossFade(int fadeOutFrames,
		int pauseFrames, int fadeInFrames,
		sf::Color c, bool skipKin = false);
	bool IsFading();
	void ClearFade();

	void SetNumGates( int nGates );
	PolyPtr GetPolygon(int index);
	
	void UpdateDecorSprites();
	void DrawBlackBorderQuads();
	void SetupMinimapBorderQuads(
		bool *blackBorder,
		bool topBorderOn);
	
	void ActivateAbsorbParticles(int absorbType, Actor *p, int storedHits,
		V2d &pos, float startAngle = 0);
	
	void UpdatePolyShaders(sf::Vector2f &botLeft,
		sf::Vector2f &playertest);
	
	void RestartLevel();
	void NextFrameRestartLevel();
	
	void SetupMapBorderQuads(
		bool *blackBorder,
		bool &topBorderOn);

	//draw map or minimap
	void DrawColoredMapTerrain(
		sf::RenderTarget *target,
		sf::Color &c);
	void DrawAllMapWires(
		sf::RenderTarget *target);

	//setup
	void SetupStormCeiling();
	void SetupGhosts(std::list<GhostEntry*> &ghosts);
	void SetupPlayers();
	void SetupShardsCapturedField();
	void SetupShaders();
	void SetupBackground();
	void SetupMinimap();
	void SetupAbsorbParticles();
	void SetupScoreDisplay();
	void SetupQuadTrees();
	bool SetupControlProfiles();
	void SetupGoalPulse();
	void SetupHUD();
	void SetupPauseMenu();
	void SetupRecGhost();
	void SetupEnergyFlow();
	sf::VertexArray *SetupBushes(int bgLayer,
		Edge *startEdge, Tileset *ts);
	sf::VertexArray * SetupPlants(Edge *start, Tileset *ts);

	//player
	int GetPlayerTeamIndex(int index = 0);
	int GetPlayerEnemiesKilledLastFrame(int index = 0);
	void PlayerRestoreDoubleJump(int index = 0);
	void PlayerRestoreAirDash(int index = 0);
	int GetPlayerHitstunFrames(int index = 0);
	bool PlayerIsMovingLeft(int index = 0);
	bool PlayerIsMovingRight(int index = 0);
	bool PlayerIsFacingRight(int index = 0);

	//draw
	void DrawHealthFlies(sf::RenderTarget *target);
	void DrawDecorBetween();
	void EnemiesCheckedMiniDraw(
		sf::RenderTarget *target,
		sf::FloatRect &rect);
	void DebugDraw();
	void DrawGoalEnergy();
	void DrawActiveEnvPlants();
	void DrawHitEnemies();
	void DrawDyingPlayers();
	void DrawTopClouds();
	void DrawRails();
	void DrawReplayGhosts();
	void DrawGates();
	void DrawEmitters(EffectLayer layer);
	void DrawActiveSequence(EffectLayer layer);
	void DrawPlayersMini(sf::RenderTarget *target);


	void UpdateDebugModifiers();
	void SetActiveSequence(Sequence *activeSeq);
	void DrawStoryLayer(EffectLayer ef);
	void SetStorySeq(StorySequence *storySeq);
	void CollectKey();
	void SuppressEnemyKeys(Gate *g);
	bool IsShardCaptured(int sType);
	void TryCreateShardResources();
	void CreateDeathSequence();
	void SetOriginalMusic();
	void UpdateTimeSlowShader();
	void UpdateEnvShaders();
	void UpdateGoalFlow();
	void UnlockPower(int pType);
	void UpdateExplodingGravityGrass();
	void AddGravityGrassToExplodeList(Grass *g);
	void RemoveGravityGrassFromExplodeList(Grass *g);
	void ResetShipSequence();
	void TriggerBarrier( Barrier *b );
	void RemoveAllEnemies();
	void UpdateEnemiesPrePhysics();
	void UpdateEnemiesPhysics();
	void UpdateEnemiesPostPhysics();
	void RecordReplayEnemies();
	void UpdateEnemiesSprites();
	void UpdateEnemiesDraw();
	void ResetEnemies();
	void SetGlobalBorders();
	void ResetPlants();
	void ResetInactiveEnemies();
	void rResetEnemies( QNode *node );
	void rResetPlants( QNode *node );
	int CountActiveEnemies();
	SaveFile *GetCurrentProgress();
	bool HasPowerUnlocked( int pIndex );
	void HandleEntrant( QuadTreeEntrant *qte );
	void FreezePlayerAndEnemies( bool freeze );
	void SoftenGates(int gCat);
	void ReformGates(int gCat );
	void OpenGates(int gCat);
	void TotalDissolveGates(int gCat);
	void ReverseDissolveGates(int gCat);
	void CloseGates(int gCat);
	void UnlockGate(Gate *g);
	void LockGate(Gate *g);
	void UpdateInput();
	void KeyboardUpdate( int index );
	bool ScreenIntersectsInversePoly( sf::Rect<double> &screenRect );
	void EndLevel();
	void AddEmitter(ShapeEmitter *emit,
		EffectLayer layer);
	void UpdateEmitters();
	void ClearEmitters();
	bool IsWithinBounds(V2d &p);
	bool IsWithinBarrierBounds(V2d &p);
	bool IsWithinCurrentBounds(V2d &p);
	void QueryBorderTree(sf::Rect<double>&rect);
	void QueryGateTree(sf::Rect<double>&rect);
};
#endif