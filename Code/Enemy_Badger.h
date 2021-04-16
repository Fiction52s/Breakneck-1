#ifndef __ENEMY_BADGER_H__
#define __ENEMY_BADGER_H__

#include "Enemy.h"

struct Badger : Enemy, GroundMoverHandler
{
	enum Action
	{
		RUN,
		LEDGEJUMP,
		SHORTJUMP,
		SHORTJUMPSQUAT,
		TALLJUMP,
		TALLJUMPSQUAT,
		ATTACK,
		LAND,
		Count
	};

	int landedAction;
	int nextAction;

	Tileset *ts;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	V2d gravity;

	int attackFrame;
	int attackMult;

	double maxGroundSpeed;
	double maxFallSpeed;

	double jumpStrength;


	Badger(ActorParams *ap);
	void ResetEnemy();
	void SetLevel(int lev);
	void ActionEnded();
	void ProcessState();
	void UpdateEnemyPhysics();
	void HandleNoHealth();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void Jump(double strengthx,
		double strengthy);
	void UpdateNextAction();
	bool StartRoll();
	void FinishedRoll();
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();
};


#endif