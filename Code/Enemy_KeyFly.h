#ifndef __ENEMY_KEYFLY_H__
#define __ENEMY_KEYFLY_H__

#include "Enemy.h"

struct KeyFly : Enemy
{
	enum Action
	{
		S_NEUTRAL,
		S_Count
	};

	KeyFly(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();

	Tileset *ts;
};

#endif