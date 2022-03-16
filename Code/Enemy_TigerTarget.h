#ifndef __ENEMY_TIGERTARGET_H__
#define __ENEMY_TIGERTARGET_H__

#include "Enemy.h"
#include "Bullet.h"

struct TigerTarget : Enemy
{
	enum Action
	{
		START_BURN,
		BURN_LOOP_HEAT_UP,
		BURN_LOOP_2,
		ATTACK_PLAYER,
		HIT_BY_PLAYER,
		ATTACK_TIGER,
		EXPLODE,
		A_Count
	};

	Tileset *ts_bigFlame;
	Tileset *ts_smallFlame;
	Tileset *ts_attack;

	int currHeatLevel;

	V2d velocity;
	double speed;
	double baseSpeed;
	double hitByPlayerSpeed;
	double maxSpeed;
	double accel;

	int maxHitByPlayerFrames;
	int currHitByPlayerFrame;

	int burnBeforeReadyFrames;

	int currBurnFrame;

	Enemy *tiger;

	TigerTarget(ActorParams *ap);

	void ProcessState();
	void UpdateEnemyPhysics();
	void IHitPlayer(int index = 0);
	void AttackPlayer();
	//void ProcessHit();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void ProcessHit();
	void ComboHit();
	bool CanComboHit( Enemy *e );
	void FrameIncrement();
	bool IsReadyToThrow();
	void SetBurnFrames(int f);


	void SetLevel(int lev);

	void UpdateSprite();
	void ResetEnemy();
};

#endif