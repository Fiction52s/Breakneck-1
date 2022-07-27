#ifndef __ENEMY_CRAWLER_H__
#define __ENEMY_CRAWLER_H__

#include "Enemy.h"

struct Crawler : Enemy, SurfaceMoverHandler
{
	enum Action : int
	{
		UNBURROW,
		DECIDE,
		CRAWL,
		ROLL,
		DASH,
		BURROW,
		ATTACK,
		UNDERGROUND,
		A_Count
	};

	SoundInfo *deathSound;
	Tileset *ts;

	int maxFramesUntilBurrow;
	int framesUntilBurrow;

	double baseSpeed;


	void SetLevel(int p_level);

	Crawler(ActorParams *ap);
	~Crawler();
	void SetActionDash();
	void DecideDirection();
	void ProcessState();
	void PlayDeathSound();
	bool TryDash();
	//void HandleNoHealth();
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
	//void UpdateEnemyPhysics();

	//Rollback
	struct MyData : StoredEnemyData
	{
		int framesUntilBurrow;
		double groundSpeed;
	};
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	
};

#endif