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

	struct MyData : StoredEnemyData
	{
		int recoverFrame;
		V2d velocity;
	};
	MyData data;

	double attentionRadius;
	double ignoreRadius;

	Tileset *ts;
	
	V2d maxSpeed;
	V2d accel;
	double rushSpeed;
	
	int recoverDuration;
	V2d testOffsetDir;

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

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif