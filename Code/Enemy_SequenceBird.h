#ifndef __ENEMY_SEQUENCEBIRD_H__
#define __ENEMY_SEQUENCEBIRD_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"

struct GameSession;

struct SequenceBird : Enemy
{
	enum Action
	{
		IDLE,
		BREATHE,
		WALK,
		FLY,
		FLY_IDLE,
		PICKUP_TIGER,
		FLY_HOLDING_TIGER,
		FLY_WITH_SKELETON,
		HIT_BY_MIND_CONTROL,
		A_Count
	};

	//Tileset *ts_walk;
	Tileset *ts;

	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d targetPos;
	int framesToArrive;

	SequenceBird(ActorParams *ap);
	void Wait();
	void Breathe();
	void Walk(V2d &pos);
	void HitByMindControl();

	void Fly(V2d &pos);
	void PickupTiger();
	void FlyAwayWithTiger(V2d &pos);
	void FlyAwayWithSkeleton(V2d &pos, double speed);

	void ProcessState();
	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void DrawMinimap(sf::RenderTarget *target) {}
	virtual bool SpawnableByCamera() { return false; }
};

#endif