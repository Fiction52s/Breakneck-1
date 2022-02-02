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
		SIMMER,
		ATTACK_PLAYER,
		HIT_BY_PLAYER,
		ATTACK_TIGER,
		EXPLODE,
		A_Count
	};

	Tileset *ts;

	int currHeatLevel;

	V2d velocity;
	double speed;
	double baseSpeed;
	double maxSpeed;
	double accel;

	Enemy *tiger;

	TigerTarget(ActorParams *ap);

	void ProcessState();
	void UpdateEnemyPhysics();
	void IHitPlayer(int index = 0);
	void HeatUp();
	//void ProcessHit();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessHit();
	void ComboHit();

	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif