#ifndef __ENEMY_CRAWLERQUEEN_H__
#define __ENEMY_CRAWLERQUEEN_H__

#include "Enemy.h"
#include "ObjectPool.h"


struct FloatingBomb : Enemy, SurfaceMoverHandler, PoolMember
{
	enum Action
	{
		FLOATING,
		EXPLODING,
		Count
	};

	Action action;
	FloatingBomb(GameSession *owner, ObjectPool *myPool, int index );
	int actionLength[Count];
	int animFactor[Count];
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	SurfaceMover *mover;
	Tileset *ts;
	HitboxInfo *hitboxInfo;
	sf::Sprite sprite;
	ObjectPool *myPool;
	void Init( V2d pos, V2d vel );
	void ProcessState();
	void HandleNoHealth();
	void HitTerrainAerial(Edge *, double);
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void ProcessHit();
};

struct CrawlerQueen : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		WAIT,
		DECIDE,
		BOOST,
		TURNAROUNDBOOST,
		STOPBOOST,
		JUMP,
		BURROW,
		RUMBLE,
		POPOUT,
		UNBURROW,
		Count
	};

	enum Decision
	{
		D_BOOST,
		D_JUMP,
		D_DIG,
		D_Count
	};

	struct EdgeInfo //for decisions
	{
		Edge *edge;
		double quantity;
		int index;
		//might need roll info?
	};


	Tileset *ts[Count];

	~CrawlerQueen();
	void HitTerrainAerial(Edge *e, double q);
	void Boost();
	void Jump();
	void Popout();
	EdgeInfo digInfo;
	int currDigAttacks;
	int digAttackCounter;
	double baseSpeed;
	double multSpeed;
	ObjectPool *bombPool;
	double bombSpeed;
	int animFactor[Count];
	int actionLength[Count];
	CrawlerQueen(GameSession *owner, Edge *ground, double quantity, bool clockwise );
	void ProcessState();
	void HandleNoHealth();
	void FrameIncrement();
	bool GetClockwise(int index);
	double GetDistanceClockwise(int index);
	double GetDistanceCCW(int index);
	void Accelerate(double amount);
	void SetForwardSpeed(double speed);
	void TransferEdge(Edge *);
	bool PlayerInFront();
	void EnemyDraw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void DecidePoints();
	void InitEdgeInfo();
	void SetDecisions();
	void DecideAction();
	void DecideNextAction( );
	//void ProcessHit();
	HitboxInfo *IsHit(Actor *player);
	int totalInvincFramesOnHit;
	int currInvincFramesOnHit;
	int invincHitThresh;
	int invincHitCount;
	bool invinc;
	int notHitFrames;
	int notHitCap;
	bool hitThisFrame;


	bool redecide;

	Edge **edgeRef;
	int numTotalEdges;

	std::map<Edge*, int> edgeIndexMap;
	bool leftInitialEdge;
	bool partLoop;
	bool completedLoop;

	sf::Sprite sprite;
	
	bool origCW;

	double groundSpeed;

	SurfaceMover *mover;

	Action action;
	bool clockwise;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;

	HitboxInfo *hitboxInfo;

	Edge *startGround;
	double startQuant;

	int decideDelayFrames;
	int decideIndex;
	int travelIndex;
	EdgeInfo *decidePoints;
	EdgeInfo startTravelPoint;
	Decision *decisions;
	const static int MAX_DECISIONS = 6;

	sf::CircleShape decDebugDraw[MAX_DECISIONS];

	int numDecisions;

	EffectPool *decMarkerPool;

	
};

#endif