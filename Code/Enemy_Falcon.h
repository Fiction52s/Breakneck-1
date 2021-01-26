#ifndef __ENEMY_FALCON_H__
#define __ENEMY_FALCON_H__

#include "Enemy.h"
#include "Bullet.h"

struct Falcon : Enemy
{
	enum Action
	{
		NEUTRAL,
		FLY,
		A_Count
	};

	double attentionRadius;
	double ignoreRadius;

	Tileset *ts;
	V2d velocity;
	double maxSpeed;
	double accel;

	Falcon(ActorParams *ap);

	void FlyMovement();
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