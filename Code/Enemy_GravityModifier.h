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

	GravityModifier(ActorParams *ap);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void AddToWorldTrees();
	//void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool Modify();
	bool IsModifiable();

	Tileset *ts;
	Tileset *ts_refresh;

	double gravFactor;

	bool increaser;
	int duration;
};

#endif