#ifndef __ENEMY_GATOR_H__
#define __ENEMY_GATOR_H__

#include "Enemy.h"
#include "Bullet.h"

struct Gator : Enemy, LauncherEnemy
{
	enum Action
	{
		DOMINATION,
		A_Count
	};

	Gator(ActorParams *ap);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(BasicBullet *b);
	void ProcessState();
	bool physicsOver;
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	int bulletSpeed;
	int framesBetween;

	BasicPathFollower pathFollower;

	Tileset *ts_bulletExplode;

	int fireCounter;

	Tileset *ts;

	int animationFactor;

	Tileset *ts_aura;
};

#endif