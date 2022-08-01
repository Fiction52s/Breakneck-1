#ifndef __ENEMY_REWINDBOOSTER_H__
#define __ENEMY_REWINDBOOSTER_H__

#include "Enemy.h"

struct RewindBooster : Enemy
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

	RewindBooster(ActorParams *ap);//sf::Vector2i &pos, int level);

	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool Boost();
	bool IsBoostable();
	void AddToWorldTrees();
	bool CountsForEnemyGate() { return false; }

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif