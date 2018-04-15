#ifndef __ENEMY_CRAWLERQUEEN_H__
#define __ENEMY_CRAWLERQUEEN_H__

#include "Enemy.h"

#ifndef __ENEMY_CRAWLER_H__
#define __ENEMY_CRAWLER_H__

#include "Enemy.h"


struct FloatingBomb : Enemy, SurfaceMoverHandler, PoolMember
{
	enum Action
	{
		FLOATING,
		EXPLODING,
		Count
	};

	Action action;
	FloatingBomb(GameSession *owner );
	int actionLength[Count];
	int animFactor[Count];
	CollisionBody *hurtBody;
	CollisionBody *hitBody;
	SurfaceMover *mover;
	Tileset *ts;
	HitboxInfo *hitboxInfo;
	sf::Sprite sprite;
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
		D_DASH,
		D_JUMP,
		D_DIG,
		D_Count
	};

	struct EdgeInfo //for decisions
	{
		Edge *edge;
		double quantity;
		int index;
	};


	~CrawlerQueen();
	int currDigAttacks;
	int digAttackCounter;
	double baseSpeed;
	double multSpeed;

	ObjectPool *bombPool;
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
	void DecideMovement();
	void InitEdgeInfo();
	
	Edge **edgeRef;
	int numTotalEdges;

	std::map<Edge*, int> edgeIndexMap;

	sf::Sprite sprite;
	Tileset *ts;
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
	EdgeInfo *decidePoints;
	Decision *decisions;

	int numDecisions;

	EffectPool *decMarkerPool;

	const static int MAX_DECISIONS = 6;
};

#endif

#endif