#ifndef __ENEMY_LOBTURRET_H__
#define __ENEMY_LOBTURRET_H__

#include "Enemy.h"
#include "Bullet.h"

struct LobTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		WAIT,
		ATTACK,
		Count
	};

	struct MyData : StoredEnemyData
	{
		int lobTypeCounter;
	};
	MyData data;

	Tileset *ts;
	Tileset *ts_bulletExplode;

	int framesWait;

	const static int NUM_LOB_TYPES = 3;
	
	V2d lobDirs[NUM_LOB_TYPES];
	double lobSpeeds[NUM_LOB_TYPES];
	bool reverse;

	//Shield *shield;

	int animationFactor;
	double bulletSpeed;

	LobTurret(ActorParams *ap);
	void UpdateHitboxes();
	void UpdateOnPlacement(ActorParams *ap);
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessState();
	void Setup();
	void SetLevel(int lev);
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void DirectKill();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult);
	void UpdateBullet(BasicBullet *b);
	void ResetEnemy();
	void FireResponse(BasicBullet *b);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif