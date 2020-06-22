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

	PoisonFrog(ActorParams *ap);
	void UpdateEnemyPhysics();
	void SetLevel(int lev);
	void UpdateHitboxes();

	Tileset *ts_test;
	double gravity;
	sf::Vector2<double> velocity;
	double angle;

	int hitsBeforeHurt;
	int hitsCounter;
	int invincibleFrames;

	double xSpeed;
	int jumpFramesWait;
	double gravityFactor;
	bool steepJump;
	sf::Vector2<double> jumpStrength;

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

	Tileset *ts_walk;
	Tileset *ts_roll;
	bool hasDoubleJump;

	double maxFallSpeed;
};

#endif