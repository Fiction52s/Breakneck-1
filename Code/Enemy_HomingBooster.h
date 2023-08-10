#ifndef __ENEMY_HOMINGBOOSTER_H__
#define __ENEMY_HOMINGBOOSTER_H__

#include "Enemy.h"

struct HomingBooster : Enemy
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

	Tileset *ts;
	Tileset *ts_refresh;

	int strength;

	HomingBooster(ActorParams *ap);//sf::Vector2i &pos, int level);
	void ProcessState();
	bool CountsForEnemyGate() { return false; }
	bool IsHomingTarget() { return false; }
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