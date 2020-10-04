#ifndef __ENEMY_CRAWLERQUEEN_H__
#define __ENEMY_CRAWLERQUEEN_H__

#include "Enemy.h"
#include "ObjectPool.h"
#include <sfeMovie/Movie.hpp>

struct Actor;
struct EffectPool;
struct StorySequence;
struct FloatingBomb : Enemy, SurfaceMoverHandler, PoolMember
{
	enum Action
	{
		FLOATING,
		EXPLODING,
		COMBO,
		Count
	};

	Action action;
	FloatingBomb( ActorParams *ap, ObjectPool *myPool, int index );
	~FloatingBomb();
	int actionLength[Count];
	int animFactor[Count];
	SurfaceMover *mover;
	Tileset *ts;
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
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void ProcessHit();
};

struct CrawlerPostFightScene;

struct CrawlerQueen : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		INITIALWAIT,
		INITIALIDLE,
		INITIALUNBURROW,
		SEQ_FINISHINITIALUNBURROW,
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
		HURT,
		RETREATBURROW,
		SEQ_ANGRY0,
		Count
	};

	enum Decision
	{
		D_BOOST,
		D_JUMP,
		D_DIG,
		D_Count
	};

	int maxDecision;
	CrawlerPostFightScene *postFightScene;
	Tileset *ts[Count];

	struct EdgeInfo //for decisions
	{
		Edge *edge;
		double quantity;
		int index;
		//might need roll info?
	};

	EdgeInfo digInfo;
	int currDigAttacks;
	int digAttackCounter;
	double baseSpeed;
	double multSpeed;
	ObjectPool *bombPool;
	double bombSpeed;
	int animFactor[Count];
	int actionLength[Count];

	int totalInvincFramesOnHit;
	int currInvincFramesOnHit;
	int invincHitThresh;
	int invincHitCount;
	bool invinc;
	int notHitFrames;
	int notHitCap;
	bool hitThisFrame;
	ObjectPool *decisionPool;
	sf::Vertex *decideVA;
	void ClearDecisionMarkers();
	Tileset *ts_decideMarker;
	int progressionLevel;
	bool redecide;

	Edge **edgeRef;
	int numTotalEdges;

	std::map<Edge*, int> edgeIndexMap;
	bool leftInitialEdge;
	bool partLoop;
	bool completedLoop;

	sf::Sprite sprite;

	GameSession *game;

	double groundSpeed;

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
	
	
	CrawlerQueen(ActorParams *ap);
	~CrawlerQueen();

	bool DecideShownAction();
	void ConfirmKill();
	void Init();
	void Setup();
	void StartInitialUnburrow();
	void StartAngryYelling();
	void StartFight();
	void InitOnRespawn();
	void SetDifficulty(int lev);
	void HitTerrainAerial(Edge *e, double q);
	void Boost();
	void Jump();
	void DeactivateAllBombs();
	void Popout();
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
	void SetDecisions();
	void DecideAction();
	void DecideNextAction( );
	HitboxInfo *IsHit(int pIndex);
};

struct DecisionMarker : PoolMember
{
	enum Action
	{
		START,
		LOOP,
		DISSIPATE,
		Count
	};

	Action action;
	void Update();
	void Reset(sf::Vector2f &pos,
		CrawlerQueen::Decision dec );
	DecisionMarker(CrawlerQueen *parent,
		ObjectPool *mPool,
		int index);
	void Clear();
	ObjectPool *myPool;
	CrawlerQueen *parent;
	sf::Vector2f pos;
	int index;
	int frame;
	CrawlerQueen::Decision dec;
};

#endif