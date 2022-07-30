#ifndef __ENEMY_PUFFERFISH_H__
#define __ENEMY_PUFFERFISH_H__

#include "Enemy.h"
#include "Bullet.h"

struct Pufferfish : Enemy, LauncherEnemy
{
	enum Action
	{
		NEUTRAL,
		PUFF,
		HOLDPUFF,
		UNPUFF,
		BLAST,
		RECOVER,
		A_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts_bulletExplode;
	int bulletSpeed;
	Tileset *ts;
	double puffRadius;
	double unpuffRadius;

	Pufferfish(ActorParams *ap);
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(int playerIndex,
		BasicBullet *b, int hitResult);
	void HandleNoHealth();
	void Fire();
	void ProcessState();
	void UpdateEnemyPhysics();
	void DirectKill();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void SetLevel(int lev);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif