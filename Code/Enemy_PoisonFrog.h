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

	PoisonFrog(GameSession *owner,
		bool hasMonitor,
		Edge *ground, double quantity,
		int level );

	int actionLength[Action::Count];
	int animFactor[Action::Count];
	//void DirectKill();
	void UpdateEnemyPhysics();

	Tileset *ts_test;

	Action action;
	bool facingRight;
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

	GroundMover *mover;

	void ProcessState();
	void ActionEnded();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void UpdateHitboxes();
	void ResetEnemy();

	void HitTerrain(double &q);
	bool StartRoll();
	void FinishedRoll();

	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();
	void HandleNoHealth();

	sf::Sprite sprite;
	Tileset *ts_walk;
	Tileset *ts_roll;
	bool hasDoubleJump;

	double maxFallSpeed;

	Edge *startGround;
	double startQuant;
};

#endif