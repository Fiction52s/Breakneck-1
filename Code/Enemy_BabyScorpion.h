#ifndef __ENEMY_BABYSCORPION_H__
#define __ENEMY_BABYSCORPION_H__

#include "Enemy.h"
#include "Bullet.h"

struct CoyoteBulletPool;

struct BabyScorpion : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		DANCE,
		DISSIPATE,
		A_Count
	};

	Tileset *ts;
	CoyoteBulletPool *stopStartPool;

	BabyScorpion(ActorParams *ap);

	void ProcessState();
	void Dance();
	void UpdateEnemyPhysics();
	void ProcessHit();
	void ActionEnded();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif