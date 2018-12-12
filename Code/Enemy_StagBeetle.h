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
		//ATTACK,
		LAND,
		Count
	};

	StagBeetle(GameSession *owner, bool hasMonitor,
		Edge *ground,
		double quantity,
		bool clockwise, double speed);
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

	sf::Sprite sprite;

	Tileset *ts;
	Tileset *ts_death;
	Tileset *ts_hop;
	Tileset *ts_idle;
	Tileset *ts_run;
	Tileset *ts_sweep;
	Tileset *ts_walk;

	Action action;
	bool facingRight;

	CubicBezier moveBezTest;
	int bezFrame;
	int bezLength;

	GroundMover *testMover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	CollisionBody *hurtBody;
	CollisionBody *hitBody;

	HitboxInfo *hitboxInfo;


	sf::Vector2<double> tempVel;
	sf::Vector2<double> gravity;


	int actionLength[Action::Count];
	int attackFrame;
	int attackMult;

	
	Edge *startGround;
	double startQuant;
	
	double angle;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	double maxGroundSpeed;
	double maxFallSpeed;
};

#endif