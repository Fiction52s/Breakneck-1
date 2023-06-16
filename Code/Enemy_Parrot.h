#ifndef __ENEMY_PARROT_H__
#define __ENEMY_PARROT_H__

#include "Enemy.h"
#include "Bullet.h"

struct Parrot : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		FLY,
		ATTACK,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		V2d velocity;
		int fireCounter;
	};
	MyData data;

	Tileset *ts;
	
	double maxSpeed;
	double accel;

	Parrot(ActorParams *ap);

	void HandleNoHealth();
	void FrameIncrement();
	void ProcessState();
	void UpdateEnemyPhysics();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();

	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex,
		BasicBullet *b, int hitResult);
	void DirectKill();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif