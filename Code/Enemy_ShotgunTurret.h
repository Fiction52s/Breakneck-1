#ifndef __ENEMY_SHOTGUNTURRET_H__
#define __ENEMY_SHOTGUNTURRET_H__

#include "Enemy.h"
#include "Bullet.h"

struct ShotgunTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		IDLE,
		ATTACK,
		WAIT,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;

	double bulletSpeed;

	ShotgunTurret(ActorParams *ap);
	void UpdateOnPlacement(ActorParams *ap);
	void ProcessState();
	void Setup();
	void SetLevel(int lev);
	void UpdateSprite();
	void UpdateBullet(BasicBullet *b);
	void ResetEnemy();
	void FireResponse(BasicBullet *b);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif