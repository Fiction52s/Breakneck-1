#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

#include "Physics.h"
#include "Tileset.h"
#include <list>
//#include "Actor.h"
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
#include "Actor.h"
#include "EffectLayer.h"

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


struct ImageText;
struct TimerText;

struct AbsorbParticles;
struct KinSkin;
struct Tileset;

struct Barrier;
struct PoiInfo
{
	PoiInfo( const std::string &name, sf::Vector2i &p );
	PoiInfo( const std::string &name, Edge *e, double q );
	sf::Vector2<double> pos;	
	Edge *edge;
	double edgeQuantity;
	std::string name;
	Barrier *barrier;
	float cameraZoom;
	bool hasCameraProperties;
};

struct Barrier
{
	Barrier( GameSession *owner, PoiInfo *poi, 
		bool p_x, int pos, bool posOp, 
		BarrierCallback *cb );

	bool Update( Actor *player );
	void SetPositive();

	BarrierCallback *callback;
	GameSession *owner;
	PoiInfo *poi;
	int pos;
	bool x; //false means y
	bool triggered;
	bool positiveOpen;
};

struct PowerBar
{
	PowerBar();

	int pointsPerDot;
	int dotsPerLine;
	int dotWidth;
	int dotHeight;
	int linesPerBar;

	int fullLines;
	int partialLine;
	//int 

	int pointsPerLayer;
	int points;
	int layer;

	int maxLayer;
	int minUse;
	sf::Sprite panelSprite;
	sf::Texture panelTex;

	int maxRecover;
	int maxRecoverLayer;

	void Reset();
	void Draw( sf::RenderTarget *target );
	bool Damage( int power );
    bool Use( int power );
	void Recover( int power );
	void Charge( int power );
};

struct PowerOrbs;
struct PowerWheel;
struct FillRing;
struct PowerRing;
struct DesperationOrb;

struct Critical : QuadTreeEntrant
{
	Critical::Critical( 
		sf::Vector2<double> &pointA, 
		sf::Vector2<double> &pointB );
	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	void Draw( sf::RenderTarget *target );

	sf::VertexArray bar;
	sf::Vector2<double> pos;
	float radius;
	sf::Vector2<double> anchorA;
	sf::Vector2<double> anchorB;
	CollisionBox box;
	bool active;

	bool hadKey[Gate::GateType::Count];

	Critical *next;
	Critical *prev;
};

struct GrassSegment
{
	CollisionBox explosion;
	GrassSegment( int edgeI, int grassIndex, int rep )
		:edgeIndex( edgeI ), index( grassIndex ), 
		reps (rep)
	{
	}
	int edgeIndex;
	int index;
	int reps;
	
};

struct Grass;

struct EnvPlant : QuadTreeEntrant
{
	EnvPlant(sf::Vector2<double>&a,
		sf::Vector2<double>&b,
		sf::Vector2<double>&c,
		sf::Vector2<double>&d,
		int vi, sf::VertexArray *v,
		Tileset *ts );
	void Reset();
	void SetupQuad();

	//Tileset *ts;
	int vaIndex;
	Tileset *ts;
	AirParticleEffect *particle;
	sf::VertexArray *va;
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D;
	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	EnvPlant *next;
	//EnvPlant *prev;
	bool activated;
	int frame;
	int idleLength;
	int idleFactor;
	int disperseLength;
	int disperseFactor;
	//EnvPlant *prev;
};


struct ScrollingBackground;
struct Boss_Crawler;
struct Boss_Bird;
struct Boss_Coyote;
struct Boss_Tiger;
struct Boss_Gator;
struct Boss_Skeleton;
struct GameSession;
struct KeyMarker;


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

struct MomentumBar
{
	MomentumBar( GameSession *owner );
	sf::Sprite teal;
	sf::Sprite blue;
	sf::Sprite purp;
	sf::Sprite container;
	sf::Sprite levelNumSpr;
	void SetTopLeft(sf::Vector2f &pos);
	int level;
	float part;
	void SetMomentumInfo(int level, float part);
	Tileset *ts_bar;
	Tileset *ts_container;
	Tileset *ts_num;
	void Draw(sf::RenderTarget *target);
	sf::Shader partShader;
};

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



struct PauseMap
{
	PauseMap();
	
};



struct SaveFile;
struct Level;
struct MainMenu;


struct DialogueUser;
struct GoalPulse;
struct PauseMenu;
struct Sequence;
struct CrawlerFightSeq;
struct CrawlerAfterFightSeq;
struct EnterNexus1Seq;
struct ShipExitSeq;




struct VictoryScreen;
struct VictoryScreen2PlayerVS;
struct UIWindow;
struct Parallax;
struct ScoreDisplay;
#include <boost/thread/mutex.hpp>

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

	void DrawStoryLayer(EffectLayer ef);
	SoundNode *ActivateSound( V2d &pos, sf::SoundBuffer *buffer, bool loop = false);
	std::map<std::string, Tileset*> decorTSMap;
	//std::map<std::string, std::list<int>> decorTileIndexes;
	std::list<DecorDraw*> decorBetween;

	StorySequence *currStorySequence;
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

		Tileset *ts_scoreTest;
		sf::Sprite scoreTestSprite;

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

	MomentumBar *momentumBar;

	sf::Vertex blackBorderQuads[4 * 2];
	sf::Vertex blackBorderQuadsMini[4 * 2];
	sf::Vertex topBorderQuadMini[4];

	HitboxManager *hitboxManager;
	AbsorbParticles *absorbParticles;
	AbsorbParticles *absorbDarkParticles;
	AbsorbParticles *absorbShardParticles;
	EnemyParamsManager *eHitParamsMan;

	static bool sLoad( GameSession *gs );
	bool Load();	
	RaceFight *raceFight;

	
	bool continueLoading;
	void SetContinueLoading( bool cont );
	bool ShouldContinueLoading();
	boost::mutex continueLoadingLock;

	MusicInfo *levelMusic;
	MusicInfo *originalMusic;
	std::map<std::string, MusicInfo*> musicMap;
	void PlayMusic(const std::string &name, sf::Time &startTime ); //add transitions later
	void TransitionMusic(const std::string &name, sf::Time &startTime,
		int crossFadeFrames);

	void StopMusic(MusicInfo *m);
	void FadeOutCurrentMusic(int numFrames);
	int musicFadeOutMax;
	int musicFadeOutCurr;
	MusicInfo *fadingOutMusic;
	MusicInfo *fadingInMusic;
	int musicFadeInMax;
	int musicFadeInCurr;


	//int playerScore[4];

	//std::map<

	Buf testBuf;
	MainMenu *mainMenu;
	RecordGhost *recGhost;

	RecordPlayer *recPlayer;
	ReplayPlayer *repPlayer;

	Tileset *ts_gravityGrass;
	Grass *explodingGravityGrass;
	void UpdateExplodingGravityGrass();
	void AddGravityGrassToExplodeList(Grass *g);
	void RemoveGravityGrassFromExplodeList(Grass *g);
	bool showDebugDraw;
	bool showTerrainDecor;
	
	bool showHUD;
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

	sf::RectangleShape fadeRect;
	int fadeFrame;
	int fadeLength;
	bool fadingIn;
	bool fadingOut;
	int fadeAlpha;
	bool fadeSkipKin;
	void Fade( bool in, int frames, sf::Color c, bool skipKin = false );
	void UpdateFade();
	void ClearFade();
	bool IsFading();
	void DrawFade( sf::RenderTarget *target );

	bool drain;
	bool shipSequence;
	sf::Vector2f cloudVel;
	int shipSeqFrame;
	sf::Vector2f shipStartPos;

	DialogueUser *activeDialogue;

	PauseMenu *pauseMenu;

	SaveFile *saveFile;
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
	std::list<Barrier*> barriers;
	
	GameSession( SaveFile *sf,
		MainMenu *mainMenu,
		const boost::filesystem::path &p_filePath );
	void TriggerBarrier( Barrier *b );
	void Init();
	void Cleanup();

	//Boss_Crawler *b_crawler;
	Boss_Crawler *b_crawler;
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
	SoundManager *soundManager;
	sf::Music *currMusic;
	sf::Texture backTex;


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
	bool LoadLights( std::ifstream &is,
		std::map<int, int> &polyIndex);
	bool LoadGates( std::ifstream &is,
		std::map<int, int> &polyIndex);
	bool LoadEnemies( std::ifstream &is,
		std::map<int, int> &polyIndex);
	std::list<Enemy*> fullEnemyList;
	void SuppressEnemyKeys( Gate::GateType gType );

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
	void UpdateTerrainShader( const sf::Rect<double> &aabb );
	void LevelSpecifics();
	bool SetGroundPar();
	void SetCloudParAndDraw();
	
	void SetUndergroundParAndDraw();
	void SetupClouds();

	void CreateZones();
	void SetupZones();
	
	int totalNumberBullets;
	sf::VertexArray *bigBulletVA;
	Tileset *ts_basicBullets;

	InputVisualizer *inputVis;
	GoalPulse *goalPulse;
	sf::VertexArray *debugBorders;

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
		float depth = 1.f);
	void AllocateLight();
	Light * ActivateLight( int radius,  int brightness, const sf::Color color );
	void DeactivateLight( Light *light );


	void RestartLevel();
	//sf::Clock inGameClock;

	void DeactivateEffect( BasicEffect *be );
	BasicEffect *inactiveEffects;

	sf::View view;

	void SaveState();
	void LoadState();

	const static int MAX_EFFECTS = 100;
	const static int MAX_DYN_LIGHTS = 1;

	std::list<MovingTerrain*> movingPlats;

	sf::Shader onTopShader;
	void SetParMountains( sf::RenderTarget *target );
	sf::Shader mountainShader;

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
	//int totalFrames; //including pausing?

	sf::Shader mountainShader1;
	void SetParMountains1( sf::RenderTarget *target );

	Tileset *ts_keyHolder;
	sf::Sprite keyHolderSprite;

	sf::VertexArray groundPar;
	sf::VertexArray underTransPar;
	sf::VertexArray undergroundPar;

	sf::VertexArray onTopPar;
	sf::Sprite closeBack0;
	Tileset *undergroundTileset;
	Tileset *undergroundTilesetNormal;
	sf::Shader underShader;

	std::list<Zone*> zones;
	Zone *currentZone;

	sf::Transform groundTrans;
	Camera cam;
	//Actor *player;
	//Actor *player2;
	Actor *players[4];
	Actor *GetPlayer( int index );
	int m_numActivePlayers;
	int numPolyTypes;
	sf::Shader *polyShaders;
	Tileset **ts_polyShaders;
	//std::map<int,sf::Shader> *polyShaderMap[
	//	TerrainPolygon::TerrainType::Count];
	sf::Shader cloneShader;
	Edge **edges;
	sf::Vector2<double> *points;

	int numGates;
	Gate **gates;
	int testGateCount;
	Gate *gateList;

	float oldZoom;
	sf::Vector2f oldCamBotLeft;
	sf::View oldView;

	Tileset *ts_leftHUD;
	Tileset *ts_speedBar;
	sf::Shader speedBarShader;
	sf::Sprite leftHUDSprite;
	sf::Sprite leftHUDBlankSprite;
	sf::Sprite speedBarSprite;

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
	
	Tileset *ts_miniCircle;
	sf::Sprite miniCircle;
	Tileset *ts_minimapGateDirection;
	sf::Sprite gateDirections[6];
	

	Background *background;

	const static int NUM_CLOUDS = 5;
	sf::Sprite clouds[NUM_CLOUDS];
	Tileset *cloudTileset;

	std::set<std::pair<int,int>> matSet;

	int xxx;

	Critical *drawCritical;
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

	
	
	//can make later children of this
	//that implement more complex behaviors
	struct DecorLayer
	{
		DecorLayer( Tileset *ts, int animLength,
			int animFactor, int tileStart = 0,
			int loopWait = 0 );
		void Update();
		Tileset *ts;
		int frame;
		int animLength;
		int animFactor;
		int startTile;
		int loopWait;
	};
	struct DecorExpression
	{
		DecorExpression( 
			std::list<sf::Vector2f> &pointList,
			DecorLayer *layer );
		~DecorExpression();

		sf::VertexArray *va;
		DecorLayer *layer;

		void UpdateSprites();
	};

	enum DecorType
	{
		D_W1_BUSH_NORMAL,
		D_W1_ROCK_1,
		D_W1_ROCK_2,
		D_W1_ROCK_3,
		D_W1_PLANTROCK,
		D_W1_VEINS1,
		D_W1_VEINS2,
		D_W1_VEINS3,
		D_W1_VEINS4,
		D_W1_VEINS5,
		D_W1_VEINS6,

		D_W1_GRASSYROCK
	};
	std::map<DecorType,DecorLayer*> decorLayerMap;



	DecorExpression * CreateDecorExpression( DecorType dType,
		int bgLayer,
		Edge *startEdge );

	std::list<DecorLayer*> DecorLayers;

	std::list<ScrollingBackground*> scrollingBackgrounds;
	//ScrollingBackground *scrollingTest;

	struct TestVA : QuadTreeEntrant
	{
		TerrainRender *tr;
		void AddDecorExpression( DecorExpression *expr );
		void UpdateBushSprites();
		void DrawBushes( sf::RenderTarget *target );
		//static int bushFrame;
		//static int bushAnimLength;
		//static int bushAnimFactor;
		//sf::VertexArray *va;
		bool visible;
		std::list<DecorExpression*> bushes;
		sf::VertexArray *groundva;
		Tileset *ts_border;
		sf::VertexArray *slopeva;
		sf::VertexArray *steepva;
		sf::VertexArray *wallva;
		sf::VertexArray *triva;
		sf::VertexArray *flowva;
		sf::VertexArray *plantva;
		sf::VertexArray *decorLayer0va;
		sf::VertexArray *bushVA;
		bool inverse;
		Tileset *ts_plant;
		Tileset *ts_terrain;
		Tileset *ts_bush; //plant = surface
		int numPoints;
		//bush = middle area
		
		sf::Shader *pShader;
		//TerrainPolygon::TerrainType terrainType;
		int terrainWorldType;
		int terrainVariation;
		//int terrainType;
		//EditSession
		//TerrainPolygon::material
		static void UpdateBushFrame();
		sf::VertexArray *terrainVA;
		sf::VertexArray *grassVA;
		bool show;
		//TestVA *prev;
		TestVA *next;
		sf::Rect<double> aabb;
		double polyArea;
		void UpdateBushes();
		void HandleQuery( QuadTreeCollider * qtc );
		bool IsTouchingBox( const sf::Rect<double> &r );
	};
	TestVA *listVA;
	std::string queryMode;

	TestVA *inversePoly;
	void SetupInversePoly( Tileset *ts_bush,
		int currentEdgeIndex );
	bool ScreenIntersectsInversePoly( sf::Rect<double> &screenRect );
	bool drawInversePoly;
	QuadTree *borderTree;
	Edge *inverseEdgeList;
	std::list<TestVA*> allVA;
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

	sf::Sprite kinMinimapIcon;
	sf::CircleShape kinMinimapTemp;
	Tileset *ts_miniIcons;
	sf::Sprite kinMapSpawnIcon;
	Tileset *ts_mapIcons;
	sf::Sprite goalMapIcon;


	Enemy *activeEnemyList;
	Enemy *activeEnemyListTail;
	Enemy *inactiveEnemyList;
	Enemy *pauseImmuneEffects;
	Enemy *cloneInactiveEnemyList;
	Enemy *effectLists[EffectLayer::Count];
	void DrawEffects( EffectLayer layer );
	void DrawActiveSequence(EffectLayer layer);

	sf::Vector2<double> originalPos;
	Zone *originalZone;
	sf::Rect<double> screenRect;
	sf::Rect<double> tempSpawnRect;
	

	QuadTree *terrainBGTree;
	QuadTree * terrainTree;
	QuadTree * enemyTree;
	QuadTree * lightTree;
	QuadTree * grassTree;
	QuadTree * gateTree;
	QuadTree * itemTree;
	QuadTree *envPlantTree;
	QuadTree *specterTree;
	QuadTree *inverseEdgeTree;
	QuadTree *staticItemTree;
	QuadTree *railEdgeTree;
	QuadTree *railDrawTree;
	QuadTree *activeItemTree; 
	QuadTree *airTriggerTree;
	std::list<AirTrigger*> fullAirTriggerList;
	
	Rail *railDrawList;

	Edge *borderEdge; 
	//for creating the outside zone

	void SetupGhosts( std::list<GhostEntry*> &ghosts );
	std::list<ReplayGhost*> replayGhosts;

	bool usePolyShader;

	//PowerBar powerBar;
	//PowerOrbs *powerOrbs;
	//PowerWheel *powerWheel;
	PowerRing *powerRing;
	DesperationOrb *despOrb;

	int pauseFrames;

	const static int MAX_LIGHTS_AT_ONCE = 16;
	int lightsAtOnce;
	Light *touchedLights[MAX_LIGHTS_AT_ONCE];
	int tempLightLimit;

	std::list<Light*> lights;
	Light *lightList;

	Light *inactiveLights;
	Light *activeLights;

	int deathWipeLength;
	int deathWipeFrame;
	bool deathWipe;

	sf::Texture wipeTextures[17];
	sf::Sprite wipeSprite;

	sf::View uiView;

	//sf::CircleShape minimapCircle;
	sf::Texture miniMaskTex;
	sf::Shader minimapShader;
	sf::RectangleShape miniRect;
	sf::VertexArray miniVA;

	sf::Sprite minimapSprite;
	//sf::VertexArray *minimapVA;
	//sf::Vector2f minimapCenter;

	sf::Texture lifeBarTex;
	sf::Sprite lifeBarSprite;

	sf::Texture miniGoalPtrTex;
	sf::Sprite miniGoalPtrSprite;
	//temp only^^

	float *fBubbleRadiusSize;
	sf::Vector2f *fBubblePos;
	float *fBubbleFrame;

	Sequence *activeSequence;
	Sequence *startMapSeq;



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

	
	//CrawlerFightSeq *crawlerFightSeq;
	//CrawlerAfterFightSeq *crawlerAfterFightSeq;
	//EnterNexus1Seq * enterNexus1Seq;

	struct Stored
	{
		Enemy *activeEnemyList;
	};
	Stored stored;

	//sf::Sprite healthSprite;

};

struct Grass : QuadTreeEntrant
{
	Grass(GameSession *p_owner, Tileset *p_ts_grass, int p_tileIndex,
		sf::Vector2<double> &pA, sf::Vector2<double> &pB,
		sf::Vector2<double> &pC, sf::Vector2<double> &pD, GameSession::TestVA *poly);

	void Reset();
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D;
	void SetVisible(bool p_visible);
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	
	void Update();
	bool exploding;
	int tileIndex;
	Grass *next;
	Grass *prev;
	bool visible;
	Tileset *ts_grass;
	CollisionBox explosion;
	int explodeFrame;
	int explodeLimit;
	GameSession *owner;
	GameSession::TestVA *poly;
	sf::IntRect aabb;
	//bool active;

	//bool prevGrass;
};



#endif