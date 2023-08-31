#ifndef __ENEMY_TIMEBOOSTER_H__
#define __ENEMY_TIMEBOOSTER_H__

#include "Enemy.h"

struct TimeBooster : Enemy
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

	TimeBooster(ActorParams *ap);//sf::Vector2i &pos, int level);
	bool CountsForEnemyGate() { return false; }
	bool IsHomingTarget() { return false; }
	void ProcessState();
	void SetLevel(int lev);
	bool IsSlowed(int index);
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