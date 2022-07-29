#ifndef __ENEMY_BOUNCEBOOSTER_H__
#define __ENEMY_BOUNCEBOOSTER_H__

#include "Enemy.h"
#include "Enemy_Booster.h"

struct BounceBooster : Enemy
{
	enum Action
	{
		NEUTRAL,
		BOOST,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;
	Tileset *ts_refresh;
	int strength;
	bool upOnly;

	bool CountsForEnemyGate() { return false; }
	BounceBooster(ActorParams *ap);	
	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();

	bool Boost();
	bool IsBoostable();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif