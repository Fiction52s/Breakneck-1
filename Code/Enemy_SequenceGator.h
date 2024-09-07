#ifndef __ENEMY_SEQUENCEGATOR_H__
#define __ENEMY_SEQUENCEGATOR_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"
#include "Enemy_GatorSuperOrb.h"

struct GameSession;

struct SequenceGator : Enemy
{
	enum Action
	{
		IDLE,
		SUPER_ORB,
		RETRACT_SUPER_ORB,
		HOLD_SUPER_ORB,
		FLOAT_WITH_ORB,
		BEAT_UP_KIN,
		SMASH_WITH_ORB,
		KICKED_BY_BIRD,
		DEAD_BODY,
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

	GatorSuperOrbPool superOrbPool;
	GatorSuperOrb *superOrb;

	V2d superOrbOffset;

	SequenceGator(ActorParams *ap);
	void Wait();
	void ThrowSuperOrb();
	void FloatWithOrb( V2d &pos, double speed );
	void SmashWithOrb(V2d &pos, double speed);
	void BeatUpKin();
	void KickedByBird();
	void LaunchSuperOrb(V2d &pos, double extraHeight, double speed);

	void ProcessState();
	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);
	void AddToGame();
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void DrawMinimap(sf::RenderTarget *target) {}
	virtual bool SpawnableByCamera() { return false; }
};

#endif