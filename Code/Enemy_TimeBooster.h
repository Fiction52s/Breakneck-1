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

	TimeBooster(ActorParams *ap);//sf::Vector2i &pos, int level);

	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool Boost();
	bool IsBoostable();
	void AddToWorldTrees();

	Tileset *ts;
	Tileset *ts_refresh;

	int strength;
};

#endif