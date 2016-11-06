#ifndef _GAMESESSION_H__
#define _GAMESESSION_H__

#include "Physics.h"
#include "Tileset.h"
#include <list>
#include "Actor.h"
#include "Enemy.h"
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
	GrassSegment( int edgeI, int grassIndex, int rep )
		:edgeIndex( edgeI ), index( grassIndex ), 
		reps (rep)
	{}
	int edgeIndex;
	int index;
	int reps;
};

struct Grass : QuadTreeEntrant
{
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D; 
	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );

	//bool prevGrass;
};

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


struct Boss_Crawler;
struct Boss_Bird;
struct Boss_Coyote;
struct Boss_Tiger;
struct Boss_Gator;
struct Boss_Skeleton;
struct GameSession;



struct KeyMarker
{
	enum State
	{
		ZERO,
		NONZERO,
		TOZERO,
		FROMZERO
	};

	int frame;
	State state;
	int keysRequired;
	int startKeys;
	GameSession *owner;
	KeyMarker( GameSession *owner );
	void SetStartKeys( int sKeys );
	void Update();
	void CollectKey();
	void SetEnergySprite();
	void Draw( sf::RenderTarget *target );
	Tileset *ts_keys;
	Tileset *ts_keyEnergy;
	sf::Sprite backSprite;
	sf::Sprite energySprite;
	//GameSession *owner;
};

struct KeyNumberObj
{
	KeyNumberObj( sf::Vector2i &p_pos, 
		int nKeys )
		:pos( p_pos ), numKeys( nKeys )
	{
	}
	sf::Vector2i pos;
	int numKeys;
};

struct ScoreDisplay
{
	ScoreDisplay( GameSession *owner,
		sf::Vector2f &position, 
		sf::Font & testFont );
	sf::Sprite score;
	void Draw( sf::RenderTarget *target );
	int numEnemiesTotal;
	int numEnemiesKilled;
	double numSeconds;
	void Reset();
	void Activate();
	void Deactivate();
	bool active;
	bool waiting;
	//void Set

	struct ScoreBar
	{
		enum State
		{
			NONE,
			POP_OUT,
			SHEET_APPEAR,
			SYMBOL_DISPLAY,
			SHEET_DISPLAY,
			RETRACT
		};

		ScoreBar( int row, ScoreDisplay *parent );
		void SetBarPos( float xDiff );
		void SetSheetFrame( int frame );
		void SetSymbolTransparency( float f );
		void ClearSheet();
		void Update();
		int row;
		State state;
		int frame;
		float xDiffPos;
		ScoreDisplay *parent;
		
	};

	void Update();

	static const int NUM_BARS = 3;
	ScoreBar *bars[NUM_BARS];
	
	GameSession *owner;
	Tileset *ts_scoreBar;
	Tileset *ts_scoreContinue;
	Tileset *ts_scoreSheet;
	Tileset *ts_scoreSymbols;

	sf::VertexArray scoreBarVA;
	sf::Sprite scoreContinue;
	sf::VertexArray scoreSymbolsVA;
	sf::VertexArray scoreSheetVA;

	sf::Vector2f basePos;
	sf::Shader colorSwapShader;
	
	sf::Text time;
	sf::Font &font;
	//sf::Sprite scoreBar;
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

struct GameSession : QuadTreeCollider, RayCastHandler
{
	enum State
	{
		RUN,
		CUTPAUSE,
		CUTSCENE,
		PAUSE,
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

	

	void ResetShipSequence(); 
	Tileset *ts_w1ShipClouds0;
	Tileset *ts_w1ShipClouds1;
	Tileset *ts_ship;
	sf::VertexArray cloud0;
	sf::VertexArray cloud1;
	sf::VertexArray cloudBot0;
	sf::VertexArray cloudBot1;

	//int shardsLoadedCounter;
	//Level *currentLevelInfo;

	sf::Vector2f relShipVel;
	//sf::Vector2f shipKinVel;
	//sf::Sprite cloud0a;
	//sf::Sprite cloud0b;
	//sf::Sprite cloud1a;
	//sf::Sprite cloud1b;
	sf::Sprite shipSprite;
	sf::RectangleShape middleClouds;

	sf::RectangleShape fadeRect;
	int fadeFrame;
	int fadeLength;
	bool fadingIn;
	bool fadingOut;
	int fadeAlpha;
	void Fade( bool in, int frames, sf::Color c);
	void UpdateFade();
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
	ScoreDisplay *scoreDisplay;
	State state;

	Tileset *ts_testParallax;
	sf::Sprite testParallaxSprite;
	
	MainMenu *mainMenu;
	KeyMarker *keyMarker;
	std::list<KeyNumberObj*> keyNumberObjects;
	
	sf::Font font;
	//int f;
	std::map<std::string,PoiInfo*> poiMap;
	std::list<Barrier*> barriers;
	//Barrier *bar2;
	//Barrier *bar;
	//int f;
	GameSession(GameController &c, 
		SaveFile *sf,
		MainMenu *mainMenu
		);
	void TriggerBarrier( Barrier *b );

	//Boss_Crawler *b_crawler;
	Boss_Crawler *b_crawler;
	Boss_Bird *b_bird;
	Boss_Coyote *b_coyote;
	Boss_Tiger *b_tiger;
	Boss_Gator *b_gator;
	Boss_Skeleton *b_skeleton;

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
	int envType;
	int envLevel;
	int substep;
	
	int keyFrame;
	sf::Font arial;

	bool cutPlayerInput;
	SoundManager *soundManager;
	sf::Music *currMusic;

	int Run( std::string fileName );
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
	
	Tileset * GetTileset( const std::string & s,
		int tileWidth, int tileHeight );
	TilesetManager tm;
	
	void Test( Edge *e );
	void AddEnemy( Enemy * e );
	void AddEffect(  EffectLayer layer, Enemy *e );
	void RemoveEffect( EffectLayer layer, Enemy *e );
	void RemoveEnemy( Enemy * e );

	void SetGlobalBorders();
	int leftBounds;
	int topBounds;
	int boundsWidth;
	int boundsHeight;
	
	void KillAllEnemies();
	void SetParOnTop(sf::RenderTarget *target );

	void UpdateEnemiesPrePhysics();
	void UpdateEnemiesPhysics();
	void UpdateEnemiesPostPhysics();
	void UpdateEffects();
	void UpdateEnemiesSprites();
	void UpdateEnemiesDraw();
	double GetTriangleArea( p2t::Triangle * t );
	void RespawnPlayer();
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
		bool right );
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
	Actor *player;
	int numPolyTypes;
	sf::Shader *polyShaders;
	Tileset **ts_polyShaders;
	//std::map<int,sf::Shader> *polyShaderMap[
	//	TerrainPolygon::TerrainType::Count];
	sf::Shader cloneShader;
	Edge **edges;
	sf::Vector2<double> *points;
	int numPoints;

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
	//sf::Sprite shockwaveSprite;

	//sf::Sprite topbarSprite;

	sf::VertexArray *va;
	ControllerState prevInput;
	ControllerState currInput;
	GameController &controller;
	Collider coll;
	std::list<sf::VertexArray*> polygons;
	std::list<sf::VertexArray*> polygonBorders;

	sf::RenderTexture *preScreenTex;
	sf::RenderTexture *postProcessTex;
	sf::RenderTexture *postProcessTex1;
	sf::RenderTexture *postProcessTex2;
	sf::RenderTexture *minimapTex;
	sf::RenderTexture *mapTex;
	sf::RenderTexture *pauseTex;

	
	
	Tileset *ts_miniCircle;
	sf::Sprite miniCircle;
	Tileset *ts_minimapGateDirection;
	sf::Sprite gateDirections[6];
	sf::Sprite background;
	sf::View bgView;



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

	sf::VertexArray * SetupPlants(
		Edge *start,
		Tileset *ts);//,
		//int (*ValidEdge)(sf::Vector2<double> &));

	void ClearFX();

	sf::Vector2<double> goalPos;
	sf::Vector2<double> goalNodePos;
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

	sf::VertexArray *SetupDecor0(
		std::vector<p2t::Triangle*> &tris,
		Tileset *ts );

	sf::VertexArray *SetupBushes( int bgLayer,
		std::vector<p2t::Triangle*> &tris, Tileset *ts );

	
	struct TestVA : QuadTreeEntrant
	{
		static int bushFrame;
		static int bushAnimLength;
		static int bushAnimFactor;
		//sf::VertexArray *va;
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
		Tileset *ts_plant;
		Tileset *ts_terrain;
		Tileset *ts_bush; //plant = surface
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
	QuadTree *borderTree;
	std::list<TestVA*> allVA;
	int numBorders;

	sf::Vector2f lastViewSize;
	sf::Vector2f lastViewCenter;
	//EdgeQNode *testTree;
	//EnemyQNode *enemyTree;
	std::string fileName;
	bool goalDestroyed;
	sf::View cloudView;

	void ActivateZone(Zone * z);
	Zone *activatedZoneList;

	void UnlockGate( Gate *g );
	void LockGate( Gate *g );
	Gate *unlockedGateList;

	sf::Sprite kinMinimapIcon;
	Tileset *ts_miniIcons;
	sf::Sprite kinMapSpawnIcon;
	Tileset *ts_mapIcons;
	sf::Sprite goalMapIcon;


	Enemy *activeEnemyList;
	Enemy *inactiveEnemyList;
	Enemy *pauseImmuneEffects;
	Enemy *cloneInactiveEnemyList;
	Enemy *effectLists[EffectLayer::Count];
	void DrawEffects( EffectLayer layer );

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
	QuadTree * crawlerReverserTree;
	QuadTree *envPlantTree;
	QuadTree *specterTree;
	CrawlerReverser *drawCrawlerReversers;
	
	//std::map<Edge*, Gate*> gateMap;

	//int numKeys;
	//Key *keys;
	//std::list<Key*> keyList;

	Edge *borderEdge; 
	//for creating the outside zone

	

	bool usePolyShader;

	PowerBar powerBar;
	//PowerOrbs *powerOrbs;
	PowerWheel *powerWheel;

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

	sf::Texture lifeBarTex;
	sf::Sprite lifeBarSprite;

	sf::Texture miniGoalPtrTex;
	sf::Sprite miniGoalPtrSprite;
	//temp only^^

	

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



#endif