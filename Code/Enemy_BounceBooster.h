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
		REFRESH,
		Count
	};

	Tileset *ts;
	Tileset *ts_refresh;
	int strength;
	bool upOnly;

	BounceBooster(ActorParams *ap);	
	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();

	bool Boost();
	bool IsBoostable();
};

#endif