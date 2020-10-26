#ifndef __ENEMY_GROWINGTREE_H__
#define __ENEMY_GROWINGTREE_H__

#include "Enemy.h"
#include "Bullet.h"

struct GrowingTree : Enemy, LauncherEnemy
{
	enum Action
	{
		RECOVER0,
		RECOVER1,
		RECOVER2,
		LEVEL0,
		LEVEL0TO1,
		LEVEL1,
		LEVEL1TO2,
		LEVEL2,
		EXPLODE,
		A_Count
	};

	GrowingTree(ActorParams *ap);
	~GrowingTree();
	void SetLevel( int lev );
	void ActionEnded();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void HandleNoHealth();
	void Fire();
	void ResetEnemy();
	void UpdateOnPlacement(ActorParams *ap);
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		V2d &pos);
	void DirectKill();
	void InitRangeMarkerVA();
	void BulletHitPlayer(
		int playerIndex,
		BasicBullet *b,
		int hitResult);

	double pulseRadius;
	int pulseFrame;
	Tileset *ts_bulletExplode;
	int powerLevel;
	int totalBullets;

	Tileset *ts;

	int startPowerLevel;

	sf::Vertex *rangeMarkerVA;
};

#endif