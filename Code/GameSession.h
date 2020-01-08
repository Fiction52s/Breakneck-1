#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

#include "Physics.h"
#include "Tileset.h"
#include <list>
//#include "Enemy.h"
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

struct LoadingMapProgressDisplay;


struct ImageText;
struct TimerText;

struct AbsorbParticles;
struct KinSkin;
struct Tileset;

struct AdventureHUD;
struct Fader;
struct Swiper;

struct Barrier;

struct BitField;

struct ShardPopup;


struct ScrollingBackground;
struct Boss_Crawler;
struct Boss_Bird;
struct Boss_Coyote;
struct Boss_Tiger;
struct Boss_Gator;
struct Boss_Skeleton;
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
struct ShipExitSeq;

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
struct EnemyParamsManager;
struct HitboxManager;
struct Background;
struct StorySequence;
struct AirTrigger;
struct Nexus;
struct ButtonHolder;

struct DecorExpression;
struct DecorLayer;
struct TouchGrassCollection;

struct HealthFly;
struct TerrainPiece;
struct SpecialTerrainPiece;

struct EnvPlant;

struct Barrier;

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

struct GameSession : QuadTreeCollider, RayCastHandler
{
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
	
	//can make later children of this
	//that implement more complex behaviors
	
	void DrawHealthFlies(sf::RenderTarget *target);
	
	sf::Vertex *healthFlyVA;
	Tileset *ts_healthFly;
	int numTotalFlies;
	std::list<HealthFly*> allFlies;

	bool hasGrass[6];
	bool hasAnyGrass;

	ShapeEmitter *testEmit;

	std::list<SpecialTerrainPiece*> allSpecialTerrain;
	ShardPopup *shardPop;
	TerrainPiece *listVA;
	SpecialTerrainPiece *specialPieceList;

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

	bool IsKeyPressed(int key);
	double accumulator;
	void Fade(bool in, int frames, sf::Color c, bool skipKin = false);
	void CrossFade(int fadeOutFrames,
		int pauseFrames, int fadeInFrames,
		sf::Color c, bool skipKin = false);
	bool IsFading();
	void ClearFade();

	Fader *fader;
	Swiper *swiper;
	Minimap *mini;
	void DrawStoryLayer(EffectLayer ef);
	SoundNode *ActivateSound( V2d &pos, sf::SoundBuffer *buffer, bool loop = false);
	std::map<std::string, Tileset*> decorTSMap;
	//std::map<std::string, std::list<int>> decorTileIndexes;
	std::list<DecorDraw*> decorBetween;
	void DrawDecorBetween();
	void DrawGoal();
	void UpdateGoalFlow();
	void DrawTerrainPieces(TerrainPiece *tPiece);
	void UpdateActiveEnvPlants();
	void DrawActiveEnvPlants();
	void UpdateDecorSprites();
	void DrawPlayerWires();
	void DrawHitEnemies();
	void DrawPlayers();
	void DrawReplayGhosts();
	void UpdateDebugModifiers();
	void DebugDraw();
	void DrawDyingPlayers();
	void UpdateTimeSlowShader();
	//void DrawActiveSequence();


	StorySequence *currStorySequence;
	void SetStorySeq(StorySequence *storySeq);
	MomentaBroadcast *currBroadcast;

	struct DecorInfo
	{
		DecorInfo(sf::Sprite &sp,
			int lay, Tileset *t, int p_tile)
			:spr(sp), layer(lay), ts( t ), tile( p_tile )
		{

		}
		sf::Sprite spr;
		int layer;
		Tileset *ts;
		int tile;
	};

	void LoadDecorImages();
	TopClouds *topClouds;
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

	
	int numTotalKeys;
	int numKeysCollected;

	sf::Vertex blackBorderQuads[4 * 4];

	HitboxManager *hitboxManager;
	AbsorbParticles *absorbParticles;
	AbsorbParticles *absorbDarkParticles;
	AbsorbParticles *absorbShardParticles;
	EnemyParamsManager *eHitParamsMan;

	bool IsShardCaptured(ShardType sType);

	static bool sLoad( GameSession *gs );
	bool Load();
	
	RaceFight *raceFight;

	
	bool continueLoading;
	void SetContinueLoading( bool cont );
	bool ShouldContinueLoading();
	boost::mutex continueLoadingLock;

	//MusicInfo *levelMusic;
	MusicInfo *originalMusic;
	std::map<std::string, MusicInfo*> musicMap;

	void UpdatePolyShaders(sf::Vector2f &botLeft,
		sf::Vector2f &playertest);
	void DrawZones();
	void DrawBlackBorderQuads();
	void DrawTopClouds();
	void UpdateEnvShaders();
	void DrawGates();
	void DrawRails();
	
	//int playerScore[4];

	//std::map<

	Buf testBuf;
	MainMenu *mainMenu;
	RecordGhost *recGhost;

	RecordPlayer *recPlayer;
	ReplayPlayer *repPlayer;

	void TryCreateShardResources();

	Tileset *ts_gravityGrass;
	Grass *explodingGravityGrass;
	void UpdateExplodingGravityGrass();
	void AddGravityGrassToExplodeList(Grass *g);
	void RemoveGravityGrassFromExplodeList(Grass *g);
	bool showDebugDraw;
	bool showTerrainDecor;
	
	AdventureHUD *adventureHUD;
	void ResetShipSequence(); 
	Tileset *ts_w1ShipClouds0;
	Tileset *ts_w1ShipClouds1;
	Tileset *ts_ship;
	sf::VertexArray cloud0;
	sf::VertexArray cloud1;
	sf::VertexArray cloudBot0;
	sf::VertexArray cloudBot1;

	ShipExitSeq *shipExitSeq;

	sf::Vector2f relShipVel;
	sf::Sprite shipSprite;
	sf::RectangleShape middleClouds;


	bool drain;
	bool shipSequence;
	bool hasShipEntrance;
	V2d shipEntrancePos;
	sf::Vector2f cloudVel;
	int shipSeqFrame;
	sf::Vector2f shipStartPos;

	DialogueUser *activeDialogue;

	PauseMenu *pauseMenu;

	SaveFile *saveFile;
	BitField *shardsCapturedField;

	sf::SoundBuffer * gameSoundBuffers[SoundType::Count];
	SoundNodeList * soundNodeList;
	SoundNodeList * pauseSoundNodeList;
	ScoreDisplay *scoreDisplay;
	State state;
	
	KeyMarker *keyMarker;
	std::list<KeyNumberObj*> keyNumberObjects;
	
	sf::Font font;
	//int f;
	std::map<std::string,PoiInfo*> poiMap;
	std::map<std::string, Barrier*> barrierMap;
	std::map<std::string, CameraShot*> cameraShotMap;

	std::list<Barrier*> barriers;
	
	GameSession( SaveFile *sf,
		MainMenu *mainMenu,
		const boost::filesystem::path &p_filePath );
	void TriggerBarrier( Barrier *b );
	void Init();
	void Cleanup();

	//Boss_Crawler *b_crawler;
	//Boss_Crawler *b_crawler;
	Boss_Bird *b_bird;
	Boss_Coyote *b_coyote;
	Boss_Tiger *b_tiger;
	Boss_Gator *b_gator;
	Boss_Skeleton *b_skeleton;

	Config *config;

	sf::VertexArray * goalEnergyFlowVA;

	~GameSession();
	void HandleRayCollision( Edge *edge, 
		double edgeQuantity, double rayPortion );
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;
	Edge *rcEdge;
	double rcQuantity;
	Edge *rayIgnoreEdge;
	Edge *rayIgnoreEdge1;
	bool quit;
	int substep;
	
	int keyFrame;
	//sf::Font arial;

	bool cutPlayerInput;
	void SetPlayerInputOn(bool on);
	SoundManager *soundManager;
	//sf::Music *currMusic;
	sf::Texture backTex;

	bool boostIntro;

	void SetOriginalMusic();


	std::map<int, bool> visibleTerrain;
	Parallax *testPar;

	virtual int Run();
	bool OpenFile( std::string fileName );
	bool LoadEdges(std::ifstream &is,
		std::map<int, int> &polyIndex);
	bool LoadBGPlats( std::ifstream &is,
		std::map<int, int> &polyIndex );
	bool LoadMovingPlats(std::ifstream &is,
		std::map<int, int> &polyIndex);
	bool LoadSpecialPolys(std::ifstream &is);
	bool LoadGates( std::ifstream &is,
		std::map<int, int> &polyIndex);
	bool LoadEnemies( std::ifstream &is,
		std::map<int, int> &polyIndex);

	bool LoadRails(std::ifstream &is);
	//void LoadEnemyOld(std::ifstream &is,
	//	std::map<int, int> &polyIndex);
	void LoadEnemy(std::ifstream &is,
		std::map<int, int> &polyIndex);
	std::list<Enemy*> fullEnemyList;
	void SuppressEnemyKeys( Gate *g );

	sf::RenderWindow *window;
	std::string currentFile;
	
	Tileset * GetTileset( const std::string & s, int tileWidth, int tileHeight,  int altColorIndex = 0 );
	Tileset * GetTileset( const std::string & s, int tileWidth, int tileHeight, int altColorIndex, int numColorChanges,
		sf::Color *startColorBuf, sf::Color *endColorBuf);
	Tileset * GetTileset(const std::string & s, int tileWidth, int tileHeight, KinSkin *skin);
	TilesetManager tm;
	
	void Test( Edge *e );
	void AddEnemy( Enemy * e );
	void AddEffect(  EffectLayer layer, Enemy *e );
	void RemoveEffect( EffectLayer layer, Enemy *e );
	void RemoveEnemy( Enemy * e );

	void SetGlobalBorders();

	
	void KillAllEnemies();
	void RemoveAllEnemies();
	void SetParOnTop(sf::RenderTarget *target );

	void UpdateEnemiesPrePhysics();
	void UpdateEnemiesPhysics();
	void UpdateEnemiesPostPhysics();
	void RecordReplayEnemies();
	void UpdateEffects();
	void UpdateEnemiesSprites();
	void UpdateEnemiesDraw();
	double GetTriangleArea( p2t::Triangle * t );
	void RespawnPlayer(int index);
	void ResetEnemies();
	void ResetPlants();
	void ResetInactiveEnemies();
	void rResetEnemies( QNode *node );
	void rResetPlants( QNode *node );
	int CountActiveEnemies();
	
	void LevelSpecifics();
	SaveFile *GetCurrentProgress();
	bool HasPowerUnlocked( int pIndex );

	void CreateZones();
	void SetupZones();
	
	int totalNumberBullets;
	sf::VertexArray *bigBulletVA;
	Tileset *ts_basicBullets;

	InputVisualizer *inputVis;
	GoalPulse *goalPulse;
	sf::VertexArray *debugBorders;

	Level *level;

	void DebugDrawActors();

	void HandleEntrant( QuadTreeEntrant *qte );
	void Pause( int frames );

	void GameStartMovie();

	void AllocateEffect();
	BasicEffect * ActivateEffect(
		EffectLayer layer,
		Tileset *ts,
		sf::Vector2<double> pos,
		bool pauseImmune,
		double angle,
		int frameCount,
		int animationFactor,
		bool right,
		int startFrame = 0,
		float depth = 1.f);

	void RestartLevel();
	void NextFrameRestartLevel();
	bool nextFrameRestart;
	//sf::Clock inGameClock;

	void DeactivateEffect( BasicEffect *be );
	BasicEffect *inactiveEffects;
	std::list<BasicEffect*> allEffectList;

	sf::View view;

	void SaveState();
	void LoadState();

	const static int MAX_EFFECTS = 100;


	std::list<MovingTerrain*> movingPlats;

	sf::Shader flowShader;
	float flowRadius;
	int flowFrameCount;
	int flowFrame;
	//float flowRadius1;
	float maxFlowRadius;
	float radDiff;
	float flowSpacing;
	float maxFlowRings;

	bool stormCeilingOn;
	double stormCeilingHeight;
	//HitboxInfo *stormCeilingInfo;

	EnvPlant *activeEnvPlants;
	int totalGameFrames;
	int totalFramesBeforeGoal;

	std::list<Zone*> zones;
	Zone *currentZone;

	sf::Transform groundTrans;
	Camera cam;
	//Actor *player;
	//Actor *player2;
	Actor *players[4];
	Actor *GetPlayer( int index );
	V2d GetPlayerPos(int index = 0);
	V2d GetPlayerKnockbackDirFromVel(int index = 0);
	int GetPlayerTeamIndex(int index = 0);
	V2d GetPlayerTrueVel(int index = 0);
	void PlayerHitGoal(int index = 0);

	int GetPlayerEnemiesKilledLastFrame(int index = 0);
	void PlayerRestoreDoubleJump(int index = 0);
	void PlayerRestoreAirDash(int index = 0);
	int GetPlayerHitstunFrames(int index = 0);
	bool PlayerIsMovingLeft(int index = 0);
	bool PlayerIsMovingRight(int index = 0);
	bool PlayerIsFacingRight(int index = 0);
	//bool PlayerHasPower(int pow);
	void PlayerAddActiveComboObj(ComboObject *, int index = 0);
	void PlayerRemoveActiveComboer(ComboObject *, int index = 0);
	void PlayerConfirmEnemyNoKill(Enemy *, int index = 0);
	void PlayerConfirmEnemyKill(Enemy *, int index = 0);
	void PlayerHitNexus(int index = 0);
	void PlayerApplyHit( HitboxInfo *hi, int index = 0);
	

	int m_numActivePlayers;
	int numPolyTypes;
	sf::Shader *polyShaders;
	Tileset **ts_polyShaders;

	TerrainDecorInfo **terrainDecorInfos;
	//std::map<int,sf::Shader> *polyShaderMap[
	//	TerrainPolygon::TerrainType::Count];
	sf::Shader cloneShader;
	Edge **edges;
	Edge *GetEdge(int index);
	std::list<Edge*> globalBorderEdges;
	sf::Vector2<double> *points;

	int numGates;
	Gate **gates;
	int testGateCount;
	Gate *gateList;
	void SoftenGates(Gate::GateType gType);
	void ReformGates(Gate::GateType gType);
	void OpenGates(Gate::GateType gType);
	void TotalDissolveGates(Gate::GateType gType);
	void ReverseDissolveGates(Gate::GateType gType);





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

	ControllerState &GetPrevInput( int index );
	ControllerState &GetCurrInput( int index );
	ControllerState &GetPrevInputUnfiltered(int index);
	ControllerState &GetCurrInputUnfiltered(int index);
	GameController &GetController( int index );
	void UpdateInput();
	void KeyboardUpdate( int index );
	void ApplyToggleUpdates( int index );
	//GameController &controller;
	//GameController *controller2;

	MapHeader *mh;
	

	Collider coll;
	std::list<sf::VertexArray*> polygons;
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
	

	Background *background;

	const static int NUM_CLOUDS = 5;
	sf::Sprite clouds[NUM_CLOUDS];
	Tileset *cloudTileset;

	std::set<std::pair<int,int>> matSet;

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

	void ClearFX();

	bool hasGoal;
	Nexus *nexus;
	
	sf::Vector2<double> goalPos;
	sf::Vector2<double> goalNodePos;
	sf::Vector2<double> goalNodePosFinal;
	V2d nexusPos;
	std::string rayMode;
	sf::VertexArray * SetupEnergyFlow1(
		int bgLayer,
		Edge *start,
		Tileset *ts );

	sf::VertexArray *SetupEnergyFlow();

	sf::VertexArray *SetupBorderTris(
		int bgLayer,
		Edge *start,
		Tileset *ts );

	sf::VertexArray *SetupTransitions(
		int bgLayer,
		Edge *start,
		Tileset *ts );
	sf::VertexArray *SetupTransitionQuads(
		int bgLayer,
		Edge *start,
		Tileset *ts);


	sf::VertexArray *SetupDecor0(
		std::vector<p2t::Triangle*> &tris,
		Tileset *ts );

	sf::VertexArray *SetupBushes( int bgLayer,
		Edge *startEdge, Tileset *ts );

	
	
	
	std::map<DecorType,DecorLayer*> decorLayerMap;



	DecorExpression * CreateDecorExpression( DecorType dType,
		int bgLayer,
		Edge *startEdge );

	std::list<DecorLayer*> DecorLayers;

	std::list<ScrollingBackground*> scrollingBackgrounds;
	//ScrollingBackground *scrollingTest;

	
	std::string queryMode;

	TerrainPiece *inversePoly;
	void SetupInversePoly( Tileset *ts_bush,
		int currentEdgeIndex );
	bool ScreenIntersectsInversePoly( sf::Rect<double> &screenRect );
	bool drawInversePoly;
	QuadTree *borderTree;
	Edge *inverseEdgeList;
	std::list<TerrainPiece*> allVA;
	int numBorders;

	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;

	std::string fileName;
	boost::filesystem::path filePath;
	bool goalDestroyed;
	GameResultType resType;
	sf::View cloudView;

	void ActivateZone(Zone * z, bool instant = false);
	Zone *activatedZoneList;

	void UnlockGate( Gate *g );
	void LockGate( Gate *g );
	Gate *unlockedGateList;

	sf::Sprite kinMapSpawnIcon;
	Tileset *ts_mapIcons;
	sf::Sprite goalMapIcon;


	Enemy *activeEnemyList;
	Enemy *activeEnemyListTail;
	Enemy *inactiveEnemyList;
	Enemy *pauseImmuneEffects;
	Enemy *cloneInactiveEnemyList;
	Enemy *effectLists[EffectLayer::Count];
	ShapeEmitter *emitterLists[EffectLayer::Count];
	void AddEmitter(ShapeEmitter *emit,
		EffectLayer layer);
	void UpdateEmitters();
	void ClearEmitters();
	void DrawEmitters(EffectLayer layer);
	void DrawEffects( EffectLayer layer );
	void DrawActiveSequence(EffectLayer layer);

	bool IsWithinBounds(V2d &p);
	bool IsWithinBarrierBounds(V2d &p);
	bool IsWithinCurrentBounds(V2d &p);

	sf::Vector2<double> originalPos;
	Zone *originalZone;
	sf::Rect<double> screenRect;
	sf::Rect<double> tempSpawnRect;
	

	QuadTree *terrainBGTree;
	QuadTree *specialTerrainTree;
	QuadTree *barrierTree;
	QuadTree * terrainTree;
	QuadTree * enemyTree;
	QuadTree * grassTree;
	QuadTree * gateTree;
	QuadTree * itemTree;
	QuadTree *envPlantTree;
	std::list<EnvPlant*> allEnvPlants;
	QuadTree *specterTree;
	QuadTree *inverseEdgeTree;
	QuadTree *staticItemTree;
	QuadTree *railEdgeTree;
	QuadTree *railDrawTree;
	QuadTree *activeItemTree;
	QuadTree *activeEnemyItemTree;
	QuadTree *airTriggerTree;
	std::list<AirTrigger*> fullAirTriggerList;
	
	Rail *railDrawList;
	int totalRails;

	Edge *borderEdge; 
	//for creating the outside zone

	void SetupGhosts( std::list<GhostEntry*> &ghosts );
	std::list<ReplayGhost*> replayGhosts;

	bool usePolyShader;

	int pauseFrames;

	int deathWipeLength;
	int deathWipeFrame;
	bool deathWipe;

	sf::View uiView;

	

	void QueryBorderTree(sf::Rect<double>&rect);
	void QueryGateTree(sf::Rect<double>&rect);
	void DrawColoredMapTerrain(
		sf::RenderTarget *target,
		sf::Color &c);
	void EnemiesCheckedMiniDraw(
		sf::RenderTarget *target,
		sf::FloatRect &rect);
	void DrawAllMapWires(
		sf::RenderTarget *target);
	void SetupMinimapBorderQuads(
		bool *blackBorder,
		bool topBorderOn);
	void SetupMapBorderQuads(
		bool *blackBorder,
		bool &topBorderOn);
	void SetupStormCeiling();

	float *fBubbleRadiusSize;
	sf::Vector2f *fBubblePos;
	float *fBubbleFrame;

	Sequence *activeSequence;
	void SetActiveSequence(Sequence *activeSeq );
	
	Sequence *startMapSeq;

	Sequence *getShardSeq;



	/*struct GameStartSeq : Sequence
	{
		GameStartSeq(GameSession *owner);
		bool Update();
		void Draw( sf::RenderTarget *target );
		sf::Texture shipTex;
		sf::Sprite shipSprite;
		sf::Texture stormTex;
		sf::Sprite stormSprite;
		sf::VertexArray stormVA;
		sf::Vector2f startPos;
		GameSession *owner;
	};
	GameStartSeq *startSeq;*/

	
	//EnterNexus1Seq * enterNexus1Seq;

	struct Stored
	{
		Enemy *activeEnemyList;
	};
	Stored stored;

	//sf::Sprite healthSprite;

};





#endif