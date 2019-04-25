#ifndef __ENEMY_ROADRUNNER_H__
#define __ENEMY_ROADRUNNER_H__

#include "Enemy.h"

struct RoadRunner : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		BURROW,
		UNDERGROUND,
		UNBURROW,
		RUN,
		STALK,
		RUSH,
		//flap
		//fly
		//glide
		Count
	};

	RoadRunner(GameSession *owner, bool hasMonitor,
		Edge *ground,
		double quantity);
	void ActionEnded();
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void FrameIncrement();

	void UpdateHitboxes();

	void ResetEnemy();

	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();

	sf::Sprite sprite;

	Tileset *ts;

	Action action;
	bool facingRight;

	GroundMover *mover;
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();


	int actionLength[Action::Count];
	int animFactor[Action::Count];

	Edge *startGround;
	double startQuant;

	int crawlAnimationFactor;
	int rollAnimationFactor;

	double maxGroundSpeed;
	double maxFallSpeed;

	int playerNotMovingRightCounter;
	int playerNotMovingLeftCounter;
	
};

#endif