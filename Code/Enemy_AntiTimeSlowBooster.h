#ifndef __ENEMY_ANTITIMESLOWBOOSTER_H__
#define __ENEMY_ANTITIMESLOWBOOSTER_H__

#include "Enemy.h"

struct AntiTimeSlowBooster : Enemy
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

	AntiTimeSlowBooster(ActorParams *ap);//sf::Vector2i &pos, int level);
	bool CountsForEnemyGate() { return false; }
	bool IsHomingTarget() { return false; }
	void ProcessState();
	bool IsSlowed(int index);
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