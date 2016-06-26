#ifndef __BOSS_H__
#define __BOSS_H__

#include "Actor.h"
#include <list>
#include "Mover.h"
#include "Movement.h"
#include "Enemy.h"

struct Boss_Crawler : Enemy, LauncherEnemy,
	SurfaceMoverHandler
{
	enum Action
	{
		SHOOT,
		BOOST
	};
	int frameTest;

	Boss_Crawler( GameSession *owner, Edge *ground, 
		double quantity );
	void ActionEnded();
	int NumTotalBullets();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	
	void SaveEnemyState();
	void LoadEnemyState();

	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );

	void HitTerrainAerial(Edge *, double);
	void TransferEdge( Edge * );

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	bool GetClockwise( int index );
	double GetDistanceClockwise( int index );
	double GetDistanceCCW( int index );

	void SetDirs();

	double totalDistanceAround;
	bool leftFirstEdge;
	Edge *firstEdge;

	Launcher *launcher;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;

	sf::VertexArray markerVA;

	//std::list<sf::Drawable*> progressDraw;

	bool onTargetEdge;
	//Tileset *ts_walk;
	//Tileset *ts_roll;
	struct EdgeInfo
	{

		Edge *edge;
		double quantity;
		//dont worry about rolling etc
		//for the boss cuz of the special room
	};
	
	int shootIndex;
	int bulletDirIndex[5];
	EdgeInfo bulletHits[5];
	int bulletIndex;
	int travelIndex;
	int numBullets;

	Action action;
	bool facingRight;

	//CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	SurfaceMover *mover;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;
	//sf::Vector2<double> gravity;
	
	int attackFrame;
	int attackMult;

	//double rollFactor;
	Contact minContact;
	bool col;
	std::string queryMode;

	int possibleEdgeCount;

	Edge *startGround;
	double startQuant;
	//sf::Vector2<double> offset;
	int frame;
	//bool roll;

	int deathFrame;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

struct Boss_Bird : Enemy, LauncherEnemy
{
	enum Action
	{
		PLANMOVE,
		MOVE,
		SHOOT
	};

	int testFrame;

	Boss_Bird( GameSession *owner, sf::Vector2i pos );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	void CreatePath();
	bool DirIsValid( sf::Vector2i &testIndex,
		sf::Vector2i &testDir );
	void UpdatePathVA();
	void ClearPathVA();

	sf::CircleShape testCircle;
	sf::CircleShape testFinalCircle;

	sf::Vector2i moveIndex;
	const sf::Vector2i DOWN;
	const sf::Vector2i LEFT;
	const sf::Vector2i RIGHT;
	const sf::Vector2i UP;
	//sf::Vector2i moveDir;
	bool moveX;
	
	sf::Vector2i finalIndex;
	const static int MAX_PATH_SIZE = 25;
	sf::Vector2i path[MAX_PATH_SIZE];
	int pathSize;

	const static int GRID_SIZE = 6;
	double gridRatio;
	double gridSizeRatio;

	int travelIndex;
	int travelFrame;
	int nodeTravelFrames;

	sf::Vector2<double> gridOriginPos;

	sf::VertexArray pathVA;

	CubicBezier moveBez;


	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	int bulletSpeed;

	Action action;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	
	sf::Vector2i originalPos;
	int frame;

	Launcher *launcher;

	int fireCounter;

	bool dying;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	//int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2<double> deathVector;
		//double deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

struct Boss_Coyote : Enemy, GroundMoverHandler
{
	enum Action
	{
		RUN,
		JUMP,
		ATTACK,
		LAND
	};

	sf::CircleShape testCircle;
	int travelFrame;
	struct ScorpionNode
	{
		ScorpionNode( sf::Vector2<double> &pos );
		ScorpionNode *neighbors[5];
		//void ResetDirection();
		void SetNewDirection();
		sf::Vector2<double> position;
		int facingIndex;
	};

	double speed;
	//void ResetDirections();
	void RandomizeDirections();
	void CreateNodes();
	ScorpionNode *points[6];
	ScorpionNode *edges[6];

	sf::VertexArray testPaths;

	ScorpionNode *currNode;
	Boss_Coyote( GameSession *owner, Edge *ground, 
		double quantity );
	void ActionEnded();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	
	void SaveEnemyState();
	void LoadEnemyState();

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	Launcher *testLaunch;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;
	//Tileset *ts_walk;
	//Tileset *ts_roll;

	Action action;
	bool facingRight;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial( Edge *e );
	void Land();

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;
	
	int attackFrame;
	int attackMult;

	//double rollFactor;
	Contact minContact;
	bool col;
	std::string queryMode;

	int possibleEdgeCount;

	Edge *startGround;
	double startQuant;
	//sf::Vector2<double> offset;
	int frame;
	//bool roll;

	int deathFrame;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;

	double maxGroundSpeed;
	double maxFallSpeed;
};

struct Boss_Tiger : Enemy, LauncherEnemy,
	SurfaceMoverHandler, RayCastHandler
{
	enum Action
	{
		GRIND,
		LUNGE
	};
	int frameTest;

	Boss_Tiger( GameSession *owner, Edge *ground, 
		double quantity );

	void HandleRayCollision( Edge *edge, double edgeQuantity, double rayPortion );
	Edge *rcEdge;
	double rcQuantity;
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;
	void ActionEnded();
	int NumTotalBullets();
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap( sf::RenderTarget *target );
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2<double> vel );
	void ResetEnemy();
	
	void SaveEnemyState();
	void LoadEnemyState();

	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );

	void HitTerrainAerial(Edge *, double);
	void TransferEdge( Edge * );

	void HitTerrain( double &q );
	bool StartRoll();
	void FinishedRoll();

	bool GetClockwise( int index );
	double GetDistanceClockwise( int index );
	double GetDistanceCCW( int index );

	//void SetDirs();

	double totalDistanceAround;
	bool leftFirstEdge;
	Edge *firstEdge;

	Launcher *launcher;
	//sf::Vector2<double> velocity;
	sf::Sprite sprite;
	Tileset *ts;

	sf::VertexArray markerVA;

	//std::list<sf::Drawable*> progressDraw;

	bool onTargetEdge;
	//Tileset *ts_walk;
	//Tileset *ts_roll;
	struct EdgeInfo
	{

		Edge *edge;
		double quantity;
		//dont worry about rolling etc
		//for the boss cuz of the special room
	};
	
	int shootIndex;
	int bulletDirIndex[5];
	EdgeInfo bulletHits[5];
	int bulletIndex;
	int travelIndex;
	int numBullets;

	Action action;
	bool facingRight;

	//CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	//CrawlerReverser *lastReverser;
	//double groundSpeed;
	//Edge *ground;
	//double edgeQuantity;
	SurfaceMover *mover;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	//CollisionBox physBody;
	HitboxInfo *hitboxInfo;
	sf::Vector2<double> tempVel;
	//sf::Vector2<double> gravity;
	
	int attackFrame;
	int attackMult;

	//double rollFactor;
	Contact minContact;
	bool col;
	std::string queryMode;

	int possibleEdgeCount;

	Edge *startGround;
	double startQuant;
	//sf::Vector2<double> offset;
	int frame;
	//bool roll;

	int deathFrame;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

struct Boss_Gator : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		CREATE_BARRIER,
		Count
	};

	const static int MAX_BARRIERS = 5;


	Boss_Gator( GameSession *owner, sf::Vector2i pos );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();

	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	int bulletSpeed;

	Action action;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	
	sf::Vector2i originalPos;
	int frame;

	Launcher *launcher;

	int fireCounter;

	bool dying;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	//int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2<double> deathVector;
		//double deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

struct Boss_Skeleton : Enemy, LauncherEnemy
{
	enum Action
	{
		PLANMOVE,
		MOVE,
		SHOOT
	};

	struct FlowerNode
	{
		FlowerNode( sf::Vector2i &pos )
			:position( pos ), numConnects( 0 )
		{
			for( int i = 0; i < 3; ++i )
				connects[i] = NULL;
			
		}

		sf::Vector2i position;
		FlowerNode *connects[3];
		
		bool connectsOpen[3];
		int numConnects;
		bool dest;
		bool partOfPath;
	};
	int testIndex;
	int testLength;
	int testFrame;
	const static int GRID_SIZE = 23;
	const static int HALF_GRID = 11;

	FlowerNode *nodes[GRID_SIZE][GRID_SIZE];

	Boss_Skeleton( GameSession *owner, sf::Vector2i pos );
	void BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos );
	void BulletHitPlayer( BasicBullet *b );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
	void Draw(sf::RenderTarget *target );
	void DrawMinimap( sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	std::pair<bool,bool> PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();
	void SaveEnemyState();
	void LoadEnemyState();
	FlowerNode * CreatePath();
	bool DirIsValid( sf::Vector2i &testIndex,
		sf::Vector2i &testDir );
	void UpdatePathVA();
	void ClearPathVA();
	FlowerNode * CreateFlowerNode( sf::Vector2i &basePos,
		int xIndex, int yIndex );
	void CreateAxisNode( sf::Vector2i &basePos,
	FlowerNode *nodes[GRID_SIZE][GRID_SIZE], 
	int xIndex, int yIndex );
	
	void CreateNode( sf::Vector2i &basePos,
		FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex,
		int yIndex);

	void CreateMirrorNode( sf::Vector2i &basePos,
		FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex,
		int yIndex);

	void CreateMirrorLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE],
		int xIndex0,
		int yIndex0, int xIndex1, int yIndex1 );

	void CreateAxisLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE],
		int xIndex0, int yIndex0, int xIndex1, int yIndex1 );

	void CreateLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE], int xIndex0,
		int yIndex0, int xIndex1, int yIndex1 );

	sf::CircleShape testCircle;
	sf::CircleShape testFinalCircle;

	sf::Vector2i moveIndex;
	const sf::Vector2i DOWN;
	const sf::Vector2i LEFT;
	const sf::Vector2i RIGHT;
	const sf::Vector2i UP;
	//sf::Vector2i moveDir;
	bool moveX;
	
	sf::Vector2i finalIndex;
	const static int MAX_PATH_SIZE = 25;
	FlowerNode *path[MAX_PATH_SIZE];
	int pathSize;

	
	double gridRatio;
	double gridSizeRatio;

	void CreateZeroLink( FlowerNode *nodes[GRID_SIZE][GRID_SIZE],
		int xIndex0, int yIndex0, int xIndex1, int yIndex1,
		int dir );

	sf::VertexArray flowerVA;
	sf::VertexArray linkVA;
	

	int travelIndex;
	int travelFrame;
	int nodeTravelFrames;

	void CreateQuadrant();
	sf::Vector2<double> gridOriginPos;

	sf::VertexArray pathVA;

	CubicBezier moveBez;
	int testNumLinks;

	std::map<Action,int> actionLength;
	std::map<Action,int> animFactor;

	int bulletSpeed;

	Action action;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	
	sf::Vector2i originalPos;
	int frame;

	Launcher *launcher;

	int fireCounter;

	bool dying;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	//int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2<double> deathVector;
		//double deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};



#endif 

