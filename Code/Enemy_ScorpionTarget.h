#ifndef __ENEMY_SCORPIONTARGET_H__
#define __ENEMY_SCORPIONTARGET_H__

#include "Enemy.h"

struct ScorpionTarget : Enemy
{
	enum Action
	{
		S_NEUTRAL,
		S_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;

	bool CountsForEnemyGate() { return false; }
	ScorpionTarget(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();
	void Collect();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);



};

#endif