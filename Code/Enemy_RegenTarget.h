#ifndef __ENEMY_REGENTARGET_H__
#define __ENEMY_REGENTARGET_H__

#include "Enemy.h"

struct RegenTarget : Enemy
{
	enum Action
	{
		A_IDLE,
		A_DYING,
		A_WAIT_BEFORE_REGEN,
		A_REGENERATING,
		A_Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;

	bool CountsForEnemyGate() { return false; }
	RegenTarget(ActorParams *ap);
	bool IsInteractible();
	bool IsValidTrackEnemy();
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();
	void Collect();
	void ProcessHit();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif