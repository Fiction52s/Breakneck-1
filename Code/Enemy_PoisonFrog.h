#ifndef __ENEMY_POISONFROG_H__
#define __ENEMY_POISONFROG_H__

#include "Enemy.h"

struct PoisonFrog : Enemy, GroundMoverHandler
{
	enum Action
	{

		STAND,
		JUMPSQUAT,
		STEEPJUMP,
		JUMP,
		LAND,
		WALLCLING,
		//STUNNED,
		Count
	};

	bool reverse;
	Tileset *ts_test;
	double gravity;
	V2d velocity;
	double angle;

	int hitsBeforeHurt;
	int hitsCounter;
	int invincibleFrames;

	double xSpeed;
	int jumpFramesWait;
	bool steepJump;
	V2d jumpStrength;

	Tileset *ts_walk;
	Tileset *ts_roll;
	bool hasDoubleJump;

	double maxFallSpeed;

	PoisonFrog(ActorParams *ap);
	void UpdateEnemyPhysics();
	void SetLevel(int lev);
	void UpdateHitboxes();

	void ProcessState();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();
	void HandleNoHealth();
};

#endif