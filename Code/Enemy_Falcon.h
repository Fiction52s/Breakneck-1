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
		RUSH,
		A_Count
	};

	double attentionRadius;
	double ignoreRadius;

	Tileset *ts;
	V2d velocity;
	V2d maxSpeed;
	V2d accel;
	double rushSpeed;
	int recoverFrame;
	int recoverDuration;

	Falcon(ActorParams *ap);

	void FrameIncrement();
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