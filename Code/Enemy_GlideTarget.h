#ifndef __ENEMY_GLIDETARGET_H__
#define __ENEMY_GLIDETARGET_H__

#include "Enemy.h"

struct GlideTarget : Enemy
{
	enum Action
	{
		S_NEUTRAL,
		S_Count
	};

	bool CountsForEnemyGate() { return false; }
	GlideTarget(ActorParams *ap);
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