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
struct Rail;
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

struct GameSession : QuadTreeCollider, RayCastHandler, Session
{
	//new stuff
	PolyPtr GetPolygon(int index);


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
	

	//new stuff from session

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
	void ProcessGate(int gType,
		int poly0Index, int vertexIndex0, int poly1Index,
		int vertexIndex1, int shardWorld,
		int shardIndex);

	void ProcessSpecialTerrain(PolyPtr poly);

	std::list<PolyPtr> allSpecialTerrain;
	PolyPtr specialPieceList;
	PolyPtr flyTerrainList;

	void SetNumGates( int nGates );

	PolyPtr inversePoly;
	std::list<PolyPtr> allPolygonsList;
	std::vector<PolyPtr> allPolysVec;
	std::map<std::string, std::list<DecorInfo>> decorListMap;
	//---------------------

	GameSession(SaveFile *sf,
		const boost::filesystem::path &p_filePath);
	~GameSession();
	

	//need to be added to session
	sf::View view;

	void Reload(const boost::filesystem::path &p_filePath);

	//shared but keep
	virtual int Run();

	//needs to be adjusted
	bool LoadEdges(std::ifstream &is,
		std::map<int, int> &polyIndex);
	bool LoadBGPlats(std::ifstream &is );
	

	bool LoadEnemies(std::ifstream &is);
	bool LoadRails(std::ifstream &is);
	//void LoadEnemy(std::ifstream &is);
	Edge *LoadEdgeIndex(std::ifstream &is);
	void LoadEdgeInfo(std::ifstream &is,
		Edge *&edge, double &edgeQuant);
	void LoadAirInfo(std::ifstream &is,
		sf::Vector2i &pos);
	void LoadNamedAirInfo(std::ifstream &is,
		sf::Vector2i &pos, std::string &str);
	void LoadStandardGroundedEnemy(std::ifstream &is,
		Edge *&edge, double &edgeQuant,
		int &hasMonitor, int &level);
	bool OpenFile();

	//TilesetManager tm;

	static GameSession *GetSession();
	static GameSession *currSession;

	//Actor *players[4]; //shared but change to vector

	std::set<std::pair<int, int>> matSet;
	bool usePolyShader;

	//anything that I already share with Session

	//anything that has to do with original terrain processing
	bool hasGrass[6];
	bool hasAnyGrass;
	PolyPtr polyQueryList;
	void UpdateDecorSprites();

	void HandleRayCollision(Edge *edge,
		double edgeQuantity, double rayPortion);
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;
	Edge *rcEdge;
	double rcQuantity;

	//these might be for the goal.
	Edge *rayIgnoreEdge;
	Edge *rayIgnoreEdge1;

	//anything that has to do with terrain but might be unique



	//anything that I might want to take for session

	void Fade(bool in, int frames, sf::Color c, bool skipKin = false);
	void CrossFade(int fadeOutFrames,
		int pauseFrames, int fadeInFrames,
		sf::Color c, bool skipKin = false);
	bool IsFading();
	void ClearFade();

	Fader *fader;
	Swiper *swiper;

	

	sf::Vertex blackBorderQuads[4 * 4];
	void DrawBlackBorderQuads();
	void SetupMinimapBorderQuads(
		bool *blackBorder,
		bool topBorderOn);


	static bool sLoad(GameSession *gs);
	bool Load();

	void ActivateAbsorbParticles(int absorbType, Actor *p, int storedHits,
		V2d &pos, float startAngle = 0);
	AbsorbParticles *absorbParticles;
	AbsorbParticles *absorbDarkParticles;
	AbsorbParticles *absorbShardParticles;

	bool continueLoading;
	void SetContinueLoading(bool cont);
	bool ShouldContinueLoading();
	boost::mutex continueLoadingLock;

	void UpdatePolyShaders(sf::Vector2f &botLeft,
		sf::Vector2f &playertest);

	void Init();
	void Cleanup();


	void RestartLevel();
	void NextFrameRestartLevel();
	bool nextFrameRestart;

	//---------------------------------
	void DrawHealthFlies(sf::RenderTarget *target);

	/*sf::Vertex *healthFlyVA;
	Tileset *ts_healthFly;
	int numTotalFlies;
	std::list<HealthFly*> allFlies;*/

	ShapeEmitter *testEmit;

	//related to decor/images not terrain related
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
	std::list<DecorDraw*> decorBetween;
	void DrawDecorBetween();

	

	Minimap *mini;
	void EnemiesCheckedMiniDraw(
		sf::RenderTarget *target,
		sf::FloatRect &rect);
	void SetupMapBorderQuads(
		bool *blackBorder,
		bool &topBorderOn);
	void DrawColoredMapTerrain(
		sf::RenderTarget *target,
		sf::Color &c);
	void DrawAllMapWires(
		sf::RenderTarget *target);


	PauseMenu *pauseMenu;
	SaveFile *saveFile;
	Config *config;
	ScoreDisplay *scoreDisplay;
	AdventureHUD *adventureHUD;

	void UpdateDebugModifiers();
	void DebugDraw();
	void DebugDrawActors();
	bool showDebugDraw;
	bool showTerrainDecor;

	Sequence *activeSequence;
	void SetActiveSequence(Sequence *activeSeq);
	BasicBossScene *preLevelScene;
	BasicBossScene *postLevelScene;
	void DrawStoryLayer(EffectLayer ef);
	StorySequence *currStorySequence;
	void SetStorySeq(StorySequence *storySeq);
	MomentaBroadcast *currBroadcast;
	DialogueUser *activeDialogue;
	
	TopClouds *topClouds;
	bool stormCeilingOn;
	double stormCeilingHeight;
	void SetupStormCeiling();

	ScreenRecorder *debugScreenRecorder;

	struct RaceFight
	{
		RaceFight( GameSession *owner, 
			int raceFightMaxSeconds );
		void Reset();
		int playerScore;
		int player2Score;
		void DrawScore( sf::RenderTarget *target );
		void UpdateScore();
		void Init();

		RaceFightHUD *hud;
		//RaceFightTarget *targetList;
		GameSession *owner;

		sf::Text tempAllTargets;
		
		ResultsScreen *victoryScreen;

		RaceFightTarget *hitByPlayerList;
		RaceFightTarget *hitByPlayer2List;
		void HitByPlayer( int playerIndex,
			RaceFightTarget *target );

		void PlayerHitByPlayer( int attacker,
			int defender );
		void TickClock();
		void TickFrame();
		int NumDigits( int number );
		void RemoveFromPlayerHitList( RaceFightTarget *target );
		void RemoveFromPlayer2HitList( RaceFightTarget *target );
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

	void CollectKey();
	KeyMarker *keyMarker;
	std::list<KeyNumberObj*> keyNumberObjects;
	int numKeysCollected;
	void SuppressEnemyKeys(Gate *g);
	
	bool IsShardCaptured(ShardType sType);
	void TryCreateShardResources();
	ShardPopup *shardPop;
	BitField *shardsCapturedField;
	Sequence *getShardSeq;

	RaceFight *raceFight;
	
	void SetOriginalMusic();
	MusicInfo *originalMusic;
	std::map<std::string, MusicInfo*> musicMap;

	sf::SoundBuffer * gameSoundBuffers[SoundType::Count];

	void UpdateTimeSlowShader();
	void UpdateEnvShaders();

	//goal
	sf::VertexArray * goalEnergyFlowVA;
	void UpdateGoalFlow();
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
	void DrawGoal();

	
	void DrawZones();
	void CreateZones();
	void SetupZones();
	void ActivateZone(Zone * z, bool instant = false);

	Zone *activatedZoneList;
	std::list<Zone*> zones;
	Zone *currentZone;
	Zone *originalZone;

	void DrawActiveEnvPlants();
	void DrawHitEnemies();
	void DrawDyingPlayers();
	void DrawTopClouds();
	

	void DrawRails();
	Rail *railDrawList;
	int totalRails;

	void UnlockPower(int pType);

	Buf testBuf;//for recording ghost
	RecordGhost *recGhost;
	RecordPlayer *recPlayer;
	ReplayPlayer *repPlayer;
	void DrawReplayGhosts();
	void SetupGhosts(std::list<GhostEntry*> &ghosts);
	std::list<ReplayGhost*> replayGhosts;
	
	Tileset *ts_gravityGrass;
	Grass *explodingGravityGrass;
	void UpdateExplodingGravityGrass();
	void AddGravityGrassToExplodeList(Grass *g);
	void RemoveGravityGrassFromExplodeList(Grass *g);

	//ship sequence. should be in its own structure
	void ResetShipSequence(); 
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

	std::map<std::string,PoiInfo*> poiMap;
	std::map<std::string, CameraShot*> cameraShotMap;

	std::map<std::string, Barrier*> barrierMap;
	std::list<Barrier*> barriers;
	void TriggerBarrier( Barrier *b );
	
	std::list<Enemy*> fullEnemyList;
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

	InputVisualizer *inputVis;
	Level *level;

	void HandleEntrant( QuadTreeEntrant *qte );
	void FreezePlayerAndEnemies( bool freeze );
	bool playerAndEnemiesFrozen;


	EnvPlant *activeEnvPlants;
	int totalGameFrames;
	int totalFramesBeforeGoal;	
	
	
	int GetPlayerTeamIndex(int index = 0);
	

	int GetPlayerEnemiesKilledLastFrame(int index = 0);
	void PlayerRestoreDoubleJump(int index = 0);
	void PlayerRestoreAirDash(int index = 0);
	int GetPlayerHitstunFrames(int index = 0);
	bool PlayerIsMovingLeft(int index = 0);
	bool PlayerIsMovingRight(int index = 0);
	bool PlayerIsFacingRight(int index = 0);
	
	
	
	

	int m_numActivePlayers;

	
	sf::Shader cloneShader; //actually time slow shader
	//Edge **edges;
	//Edge *GetEdge(int index);
	std::list<Edge*> globalBorderEdges;
	//sf::Vector2<double> *points;

	void DrawGates();
	int numGates;
	std::vector<Gate*> gates;
	int testGateCount;
	Gate *gateList;
	void SoftenGates(Gate::GateType gType);
	void ReformGates(Gate::GateType gType);
	void OpenGates(Gate::GateType gType);
	void TotalDissolveGates(Gate::GateType gType);
	void ReverseDissolveGates(Gate::GateType gType);
	void CloseGates(Gate::GateType gType);
	void UnlockGate(Gate *g);
	void LockGate(Gate *g);
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
	//sf::Sprite shockwaveSprite;

	//sf::Sprite topbarSprite;

	sf::VertexArray *va;

	
	void UpdateInput();
	void KeyboardUpdate( int index );
	

	Collider coll;
	
	//std::list<sf::VertexArray*> polygons;
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

	

	static int IsFlatGround( sf::Vector2<double> &normal );
	static int IsSlopedGround( sf::Vector2<double> &normal );
	static int IsSteepGround(  sf::Vector2<double> &normal );
	static int IsWall( sf::Vector2<double> &normal );
	sf::VertexArray * SetupBorderQuads(
		int bgLayer,
		Edge *start,
		//int currentEdgeIndex,
		Tileset *ts,
		int (*ValidEdge)(sf::Vector2<double> &)
		);
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
	sf::VertexArray * SetupBorder(
		int bgLayer,
		Edge *start,
		Tileset *ts,
		int(*ValidEdge)(sf::Vector2<double> &)
	);

	sf::VertexArray * SetupPlants(
		Edge *start,
		Tileset *ts);//,
		//int (*ValidEdge)(sf::Vector2<double> &));

	
	
	//bool shaders
	bool shadersLoaded;
	
	std::string rayMode;

	void SetupPlayers();

	void SetupKeyMarker();

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

	sf::VertexArray *SetupBushes( int bgLayer,
		Edge *startEdge, Tileset *ts );

	std::map<DecorType,DecorLayer*> decorLayerMap;

	std::list<DecorLayer*> DecorLayers;
	
	std::string queryMode;


	bool ScreenIntersectsInversePoly( sf::Rect<double> &screenRect );
	bool drawInversePoly;
	Edge *inverseEdgeList;
	int numBorders;

	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;

	
	bool goalDestroyed;

	//void EndLevel(GameResultType rType);
	void EndLevel();

	GameResultType resType;
	sf::View cloudView;

	

	

	sf::Sprite kinMapSpawnIcon;
	Tileset *ts_mapIcons;
	//sf::Sprite goalMapIcon;

	ShapeEmitter *emitterLists[EffectLayer::Count];
	void AddEmitter(ShapeEmitter *emit,
		EffectLayer layer);
	void UpdateEmitters();
	void ClearEmitters();
	void DrawEmitters(EffectLayer layer);
	void DrawActiveSequence(EffectLayer layer);

	bool IsWithinBounds(V2d &p);
	bool IsWithinBarrierBounds(V2d &p);
	bool IsWithinCurrentBounds(V2d &p);

	//sf::Vector2<double> originalPos;
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

	void QueryBorderTree(sf::Rect<double>&rect);
	void QueryGateTree(sf::Rect<double>&rect);
};





#endif