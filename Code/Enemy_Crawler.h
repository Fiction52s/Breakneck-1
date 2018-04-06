#ifndef __ENEMY_CRAWLER_H__
#define __ENEMY_CRAWLER_H__

#include "Enemy.h"

struct Crawler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		UNBURROW,
		CRAWL,
		STARTROLL,
		ROLL,
		ENDROLL,
		DASH,
		BURROW,
		UNDERGROUND,
		DYING
	};

	int actionLength[DYING+1];
	Crawler(GameSession *owner, bool hasMonitor, Edge *ground, double quantity, bool clockwise, int speed, int framesUntilBurrow );
	void SetActionDash();
	void ProcessState();
	bool TryDash();
	void HandleNoHealth();
	void FrameIncrement();
	bool IsPlayerChasingMe();
	void AttemptRunAwayBoost();
	double dashAccel;
	void Accelerate(double amount);
	void SetForwardSpeed( double speed );
	void TransferEdge(Edge *);

	bool ShouldDash();
	bool PlayerInFront();
	void EnemyDraw(sf::RenderTarget *target);
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void ResetEnemy();
	void UpdateEnemyPhysics();

	sf::Sprite sprite;
	double totalDistBeforeBurrow;
	double currDistTravelled;
	Tileset *ts;
	bool origCW;

	int maxFramesUntilBurrow;
	int framesUntilBurrow;

	double groundSpeed;

	SurfaceMover *mover;

	Action action;
	bool clockwise;

	CollisionBody *hurtBody;
	CollisionBody *hitBody;

	HitboxInfo *hitboxInfo;
	

	Edge *startGround;
	double startQuant;

	int crawlAnimationFactor;
	int rollAnimationFactor;
};

#endif