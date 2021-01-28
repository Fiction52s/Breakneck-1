#ifndef __ENEMY_FUTURECHECKER_H__
#define __ENEMY_FUTURECHECKER_H__

#include "Enemy.h"
#include "Bullet.h"

struct FutureChecker : Enemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		INVISIBLE,
		FADEIN,
		FADEOUT,
		A_Count
	};

	Tileset *ts_bulletExplode;
	int bulletSpeed;

	int fireCounter;

	Tileset *ts;

	V2d playerTrackPos;
	V2d velocity;
	double accel;
	double maxSpeed;

	FutureChecker(ActorParams *ap);
	void HandleNoHealth();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif