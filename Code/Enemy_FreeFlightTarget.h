#ifndef __ENEMY_FREEFLIGHTTARGET_H__
#define __ENEMY_FREEFLIGHTTARGET_H__

#include "Enemy.h"

struct FreeFlightTarget : Enemy
{
	enum Action
	{
		S_NEUTRAL,
		S_Count
	};

	FreeFlightTarget(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void AddToWorldTrees();
	void Collect();

	Tileset *ts;
};

#endif