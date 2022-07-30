#ifndef __ENEMY_GRAVITYMODIFIER_H__
#define __ENEMY_GRAVITYMODIFIER_H__

#include "Enemy.h"

struct GravityModifier : Enemy
{
	enum Action
	{
		NEUTRAL,
		MODIFY,
		REFRESH,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;
	Tileset *ts_refresh;

	double gravFactor;

	bool increaser;
	int duration;

	
	GravityModifier(ActorParams *ap);
	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void AddToWorldTrees();
	//void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool Modify();
	bool IsModifiable();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	
};

#endif