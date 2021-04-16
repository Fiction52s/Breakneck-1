#ifndef __ENEMY_STAGBEETLE_H__
#define __ENEMY_STAGBEETLE_H__

#include "Enemy.h"

struct StagBeetle : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		RUN,
		JUMP,
		LAND,
		Count
	};

	StagBeetle(ActorParams *ap);
	void HandleNoHealth();
	void ActionEnded();
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	
	void DebugDraw(sf::RenderTarget *target);
	void UpdateSprite();
	
	void ResetEnemy();

	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();
	void SetLevel(int lev);
	bool IsFacingTrueRight();

	Shield *shield;

	Tileset *ts_death;
	Tileset *ts_hop;
	Tileset *ts_idle;
	Tileset *ts_run;
	Tileset *ts_sweep;
	Tileset *ts_walk;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	bool reverse;

	sf::Vector2<double> tempVel;
	double gravity;

	int attackFrame;
	int attackMult;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	double maxGroundSpeed;
	double maxFallSpeed;
};

#endif