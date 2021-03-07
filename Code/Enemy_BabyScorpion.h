#ifndef __ENEMY_BABYSCORPION_H__
#define __ENEMY_BABYSCORPION_H__

#include "Enemy.h"
#include "Bullet.h"

struct BabyScorpion : Enemy
{
	enum Action
	{
		NEUTRAL,
		DISSIPATE,
		A_Count
	};

	Tileset *ts;

	BabyScorpion(ActorParams *ap);

	void ProcessState();
	void UpdateEnemyPhysics();
	void ProcessHit();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif