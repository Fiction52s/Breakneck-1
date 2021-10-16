#ifndef __ENEMY_SEQUENCECRAWLER_H__
#define __ENEMY_SEQUENCECRAWLER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "EnemyMover.h"

struct GameSession;

struct SequenceCrawler : Enemy
{
	enum Action
	{
		IDLE,
		UNDERGROUND,
		DIG_OUT,
		DIG_IN,
		TRIGGER_BOMBS,
		HIT_BY_TIGER,
		DYING_BREATH,
		DIE_BY_TIGER,
		DEAD,
		A_Count
	};

	//Tileset *ts_walk;
	Tileset *ts_dig_in;
	Tileset *ts_dig_out;

	int moveFrames;
	int waitFrames;

	EnemyMover enemyMover;

	int targetPlayerIndex;

	V2d targetPos;
	int framesToArrive;

	SequenceCrawler(ActorParams *ap);
	void Wait();
	void DigOut();
	void DigIn();
	void Underground();
	void TriggerBombs();
	void HitByTiger();
	void DieByTiger();

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