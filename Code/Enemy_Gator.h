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

	struct MyData : StoredEnemyData
	{
		int fireCounter;
	};

	int fireCounter;
	
	Tileset *ts;
	Tileset *ts_bulletExplode;
	Tileset *ts_aura;
	int animationFactor;
	int bulletSpeed;
	int framesBetween;

	Gator(ActorParams *ap);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge, V2d &pos);
	void BulletHitPlayer(
		int playerIndex, 
		BasicBullet *b,
		int hitResult);
	void ProcessState();
	
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();

	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();
};

#endif