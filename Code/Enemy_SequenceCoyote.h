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
		RUN,
		SUMMON_SCORPION,
		SCORPION_STAND,
		TURN,
		JUMPSQUAT,
		BOUNCE,
		SLEEP,
		HOPSQUAT,
		HOP_BACK,
		A_Count
	};

	Tileset *ts_coy;
	Tileset *ts_scorp;

	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d bouncePos;
	V2d hopPos;
	V2d targetPos;
	int framesToArrive;

	sf::Sprite scorpSprite;
	

	SequenceCoyote(ActorParams *ap);

	virtual bool SpawnableByCamera() { return false; }
	void Wait();
	void Run(V2d &pos);
	void Walk(V2d &pos);
	void SummonScorpion();
	void Bounce(V2d &pos);
	void Sleep();
	void HopBack(V2d &pos);
	//void WakeUpFromSleep();

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