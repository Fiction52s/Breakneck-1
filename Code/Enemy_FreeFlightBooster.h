#ifndef __ENEMY_FREEFLIGHTBOOSTER_H__
#define __ENEMY_FREEFLIGHTBOOSTER_H__

#include "Enemy.h"

struct FreeFlightBooster : Enemy
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
	SoundInfo *boostSound;

	int strength;

	FreeFlightBooster(ActorParams *ap);//sf::Vector2i &pos, int level);
	bool CountsForEnemyGate() { return false; }
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