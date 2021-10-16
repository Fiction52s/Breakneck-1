#ifndef __ENEMY_SEQUENCETIGER_H__
#define __ENEMY_SEQUENCETIGER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"

struct GameSession;

struct SequenceTiger : Enemy
{
	enum Action
	{
		IDLE,
		WALK,
		BREATHE,
		LOOK_UP,
		CARRIED_BY_BIRD,
		HIT_BY_MIND_CONTROL,
		INJURED_ROAR,
		FALL,
		FALL_LAND_IDLE,
		PUT_BIRD_ON_BACK,
		CARRY_BIRD,
		LUNGE,
		PRE_CRAWLER_KILL,
		CRAWLER_KILL,
		A_Count
	};

	//Tileset *ts_walk;
	Tileset *ts_roar;
	Tileset *ts_walk;

	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d targetPos;
	int framesToArrive;

	double extraHeight;

	SequenceTiger(ActorParams *ap);
	void Wait();

	void Lunge(V2d &pos, double extraHeight, double speed);
	void Fall(double y );
	void InjuredRoar();
	void HitByMindControl();
	void Walk(V2d &pos);
	void LookUp();
	void Carried(V2d &pos);
	void Breathe();
	void CarryBirdAway(V2d &pos);
	void PutBirdOnBack();
	void KillCrawler();

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