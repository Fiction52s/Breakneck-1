#ifndef __ENEMY_GROWINGTREE_H__
#define __ENEMY_GROWINGTREE_H__

#include "Enemy.h"
#include "Bullet.h"

struct GrowingTree : Enemy, LauncherEnemy
{
	enum Action
	{
		IDLE,
		ATTACK,
		/*NEUTRAL0,
		NEUTRAL1,
		NEUTRAL2,
		ATTACK0,
		LEVEL0TO1,
		ATTACK1,
		LEVEL1TO2,
		ATTACK2,*/
		A_Count
	};

	struct MyData : StoredEnemyData
	{
	};
	MyData data;

	double pulseRadius;
	int pulseFrame;
	Tileset *ts_bulletExplode;
	
	int totalBullets;
	Tileset *ts;
	int startPowerLevel;
	
	int repsToLevelUp;

	GrowingTree(ActorParams *ap);
	~GrowingTree();
	void SetLevel( int lev );
	void ActionEnded();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void HandleNoHealth();
	void ResetEnemy();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		V2d &pos);
	void UpdateBullet(BasicBullet *b);
	void DirectKill();
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif