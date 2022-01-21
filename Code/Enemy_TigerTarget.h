#ifndef __ENEMY_TIGERTARGET_H__
#define __ENEMY_TIGERTARGET_H__

#include "Enemy.h"
#include "Bullet.h"

struct TigerTarget : Enemy
{
	enum Action
	{
		NEUTRAL,
		HEAT_UP,
		ATTACK_TIGER,
		ATTACK_PLAYER,
		A_Count
	};

	Tileset *ts;

	TigerTarget(ActorParams *ap);

	void ProcessState();
	void UpdateEnemyPhysics();
	//void ProcessHit();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif