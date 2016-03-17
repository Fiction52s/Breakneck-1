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

	bool hadBlueKey;

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

struct GameSession : QuadTreeCollider, RayCastHandler
{
	GameSession(GameController &c, sf::RenderWindow *rw, 
		sf::RenderTexture *preTex,
		sf::RenderTexture *miniTex);

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

	sf::RenderWindow *window;
	std::string currentFile;
	std::list<Tileset*> tilesetList;
	Tileset * GetTileset( const std::string & s,
		int tileWidth, int tileHeight );
	void Test( Edge *e );
	void AddEnemy( Enemy * e );
	void RemoveEnemy( Enemy * e );

	void SetGlobalBorders();
	int leftBounds;
	int topBounds;
	int boundsWidth;
	int boundsHeight;
	
	void SetParOnTop(sf::RenderTarget *target );

	void UpdateEnemiesPrePhysics();
	void UpdateEnemiesPhysics();
	void UpdateEnemiesPostPhysics();
	void UpdateEnemiesSprites();
	void UpdateEnemiesDraw();
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
	

	sf::VertexArray *debugBorders;

	void DebugDrawActors();

	void HandleEntrant( QuadTreeEntrant *qte );
	void Pause( int frames );

	void GameStartMovie();

	void AllocateEffect();
	BasicEffect * ActivateEffect( 
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

	sf::Transform groundTrans;
	Camera cam;
	Actor player;
	sf::Shader polyShader;
	sf::Shader cloneShader;
	Edge **edges;
	sf::Vector2<double> *points;
	int numPoints;

	int numGates;
	Gate **gates;
	int testGateCount;
	Gate *gateList;

	Tileset *topbar;
	sf::Sprite topbarSprite;

	sf::VertexArray *va;
	ControllerState prevInput;
	ControllerState currInput;
	GameController &controller;
	Collider coll;
	std::list<sf::VertexArray*> polygons;
	std::list<sf::VertexArray*> polygonBorders;

	sf::RenderTexture *preScreenTex;
	sf::RenderTexture *minimapTex;
	sf::Sprite background;
	sf::View bgView;

	const static int NUM_CLOUDS = 5;
	sf::Sprite clouds[NUM_CLOUDS];
	Tileset *cloudTileset;

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

	sf::Vector2<double> goalPos;
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

	struct TestVA : QuadTreeEntrant
	{
		//sf::VertexArray *va;
		sf::VertexArray *groundva;
		Tileset *ts_border;
		sf::VertexArray *slopeva;
		sf::VertexArray *steepva;
		sf::VertexArray *wallva;
		sf::VertexArray *triva;
		sf::VertexArray *flowva;
		sf::VertexArray *plantva;
		Tileset *ts_plant;
		
		
		sf::VertexArray *terrainVA;
		sf::VertexArray *grassVA;
		bool show;
		//TestVA *prev;
		TestVA *next;
		sf::Rect<double> aabb;
		void HandleQuery( QuadTreeCollider * qtc );
		bool IsTouchingBox( const sf::Rect<double> &r );
	};
	TestVA *listVA;
	std::string queryMode;
	QuadTree *borderTree;
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



	Enemy *activeEnemyList;
	Enemy *inactiveEnemyList;
	Enemy *pauseImmuneEffects;
	Enemy *cloneInactiveEnemyList;

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
	CrawlerReverser *drawCrawlerReversers;
	
	//std::map<Edge*, Gate*> gateMap;

	//int numKeys;
	//Key *keys;
	std::list<Key*> keyList;

	Edge *borderEdge; 
	//for creating the outside zone

	

	bool usePolyShader;

	PowerBar powerBar;

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

	struct Sequence
	{
		//Sequence *next;
		//Sequence *prev;
		int frameCount;
		int frame;
		virtual bool Update() = 0;
		virtual void Draw( sf::RenderTarget *target ) = 0;
	};

	Sequence *activeSequence;

	struct GameStartSeq : Sequence
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
	GameStartSeq *startSeq;


	struct Stored
	{
		Enemy *activeEnemyList;
	};
	Stored stored;

	//sf::Sprite healthSprite;

};



#endif