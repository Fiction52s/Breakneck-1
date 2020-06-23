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
	
	void UpdateHitboxes();
	
	void ResetEnemy();

	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();
	void SetLevel(int lev);

	Shield *shield;

	Tileset *ts_death;
	Tileset *ts_hop;
	Tileset *ts_idle;
	Tileset *ts_run;
	Tileset *ts_sweep;
	Tileset *ts_walk;

	Tileset *ts_hopAura;
	Tileset *ts_idleAura;
	Tileset *ts_runAura;
	Tileset *ts_sweepAura;
	Tileset *ts_walkAura;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;

	int attackFrame;
	int attackMult;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	double maxGroundSpeed;
	double maxFallSpeed;
};

#endif