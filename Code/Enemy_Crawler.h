#ifndef __ENEMY_CRAWLER_H__
#define __ENEMY_CRAWLER_H__

#include "Enemy.h"

struct Crawler : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		UNBURROW,
		DECIDE,
		CRAWL,
		ROLL,
		DASH,
		BURROW,
		ATTACK,
		UNDERGROUND
	};

	static Enemy *Create(ActorParams *ap);

	int animFactor[UNDERGROUND + 1];
	int actionLength[UNDERGROUND +1];
	Crawler(bool hasMonitor, Edge *ground, double quantity, int level);
	~Crawler();
	void SetActionDash();
	void DecideDirection();
	void ProcessState();
	void PlayDeathSound();
	bool TryDash();
	void HandleNoHealth();
	void DecideMovement();
	void FrameIncrement();
	bool IsPlayerChasingMe();
	bool ShouldAttack();
	bool TryAttack();
	bool TryDashAndAttack();
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

	sf::SoundBuffer *deathSound;
	sf::Sprite sprite;
	sf::Sprite auraSprite;
	double totalDistBeforeBurrow;
	double currDistTravelled;
	Tileset *ts;
	Tileset *ts_aura;
	bool origCW;

	int maxFramesUntilBurrow;
	int framesUntilBurrow;

	double groundSpeed;

	SurfaceMover *mover;

	Action action;
	bool clockwise;
	

	Edge *startGround;
	double startQuant;

	int crawlAnimationFactor;
	int rollAnimationFactor;
};

#endif