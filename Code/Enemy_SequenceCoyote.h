#ifndef __ENEMY_SEQUENCECOYOTE_H__
#define __ENEMY_SEQUENCECOYOTE_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"

struct GameSession;

struct SequenceCoyote : Enemy
{
	enum Action
	{
		IDLE,
		WAIT,
		WALK,
		SUMMON_SCORPION,
		SCORPION_STAND,
		TURN,
		JUMPSQUAT,
		BOUNCE,
		A_Count
	};

	Tileset *ts_coy;
	Tileset *ts_scorp;

	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d bouncePos;
	V2d targetPos;
	int framesToArrive;

	sf::Sprite scorpSprite;
	

	SequenceCoyote(ActorParams *ap);

	virtual bool SpawnableByCamera() { return false; }
	void Wait();
	void Walk(V2d &pos);
	void SummonScorpion();
	void Bounce(V2d &pos);

	void ProcessState();
	void DebugDraw(sf::RenderTarget *target);

	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void DrawMinimap(sf::RenderTarget *target) {}
};

#endif