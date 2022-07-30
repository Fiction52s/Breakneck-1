#ifndef __ENEMY_ROADRUNNER_H__
#define __ENEMY_ROADRUNNER_H__

#include "Enemy.h"

struct Roadrunner : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		WAKEUP,
		RUN,
		JUMP,
		LAND,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	Tileset *ts;
	double gravity;
	double maxGroundSpeed;
	double maxFallSpeed;
	double runAccel;
	double runDecel;
	double attentionRadius;
	double ignoreRadius;

	Roadrunner(ActorParams *ap);
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
	void HitOther();
	void ReachCliff();
	void HitOtherAerial(Edge *e);
	void Land();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif