#ifndef __ENEMY_POISONFROG_H__
#define __ENEMY_POISONFROG_H__

#include "Enemy.h"

struct PoisonFrog : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		WALL_IDLE,
		STAND,
		JUMPSQUAT,
		STEEPJUMP,
		JUMP,
		LAND,
		WALLCLING,
		//STUNNED,
		Count
	};

	struct MyData : StoredEnemyData
	{
		int invincibleFrames;
		bool hasDoubleJump;
	};
	MyData data;

	bool reverse;
	Tileset *ts_test;
	double gravity;
	double angle;

	int hitsBeforeHurt;
	int hitsCounter;
	

	double xSpeed;
	bool steepJump;
	V2d jumpStrength;

	Tileset *ts_walk;
	Tileset *ts_roll;
	

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

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif