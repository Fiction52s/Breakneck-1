#ifndef __ENEMY_CHEETAH_H__
#define __ENEMY_CHEETAH_H__

#include "Enemy.h"

struct Cheetah : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		RUN,
		CHARGE,
		BOOST,
		A_Count
	};

	double attentionRadius;
	double ignoreRadius;
	int preChargeFrames;
	int preChargeLimit;
	double boostSpeed;

	double runAccel;
	double runDecel;
	double turnaroundDist;
	double boostPastDist;

	Action landedAction;
	Action nextAction;

	Tileset *ts;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;

	int attackFrame;
	int attackMult;

	double maxGroundSpeed;
	double maxFallSpeed;

	double jumpStrength;

	Tileset *ts_aura;

	Cheetah(ActorParams *ap);
	void RunMovement();
	void ActionEnded();
	void ProcessState();
	void SetLevel(int lev);
	void UpdateEnemyPhysics();
	void HandleNoHealth();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();
	void Jump(double strengthx,
		double strengthy);
	bool StartRoll();
	void FinishedRoll();
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();
};


#endif