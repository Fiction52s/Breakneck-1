#ifndef __ENEMY_CURVETURRET_H__
#define __ENEMY_CURVETURRET_H__

#include "Enemy.h"
#include "Bullet.h"

struct CurveTurret : Enemy, LauncherEnemy
{
	enum Action
	{
		WAIT,
		ATTACK,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	
	int framesWait;
	int realWait;

	double spins[16];
	int startBulletIndex;

	Shield *shield;

	double angle;

	Tileset *ts;
	Tileset *ts_bulletExplode;

	double bulletSpeed;
	int turnFactor;
	int animationFactor;

	CurveTurret(ActorParams *ap);
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
		int hitResult );
	void UpdateBullet(BasicBullet *b);
	void ResetEnemy();
	void FireResponse(BasicBullet *b);	

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif