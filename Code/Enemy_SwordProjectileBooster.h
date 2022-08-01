#ifndef __ENEMY_SWORDPROJECTILEBOOSTER_H__
#define __ENEMY_SWORDPROJECTILEBOOSTER_H__

#include "Enemy.h"

struct SwordProjectileBooster : Enemy
{
	enum Action
	{
		NEUTRAL,
		BOOST,
		REFRESH,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	bool enemyProjectile;

	Tileset *ts;
	Tileset *ts_refresh;

	int strength;

	SwordProjectileBooster(ActorParams *ap);//sf::Vector2i &pos, int level);

	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool Boost();
	bool IsBoostable();
	void AddToWorldTrees();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif