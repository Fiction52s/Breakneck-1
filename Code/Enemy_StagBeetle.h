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

	struct MyData : StoredEnemyData
	{
	};
	MyData data;

	Shield *shield;

	Tileset *ts_death;
	Tileset *ts_hop;
	Tileset *ts_idle;
	Tileset *ts_run;
	Tileset *ts_sweep;
	Tileset *ts_walk;

	bool reverse;

	V2d tempVel;
	double gravity;

	
	int attackMult;
	int crawlAnimationFactor;
	int rollAnimationFactor;

	double maxGroundSpeed;
	double maxFallSpeed;

	StagBeetle(ActorParams *ap);
	~StagBeetle();
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
	void UpdateHitboxes();
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif