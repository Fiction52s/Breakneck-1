#ifndef __ENEMY_BADGER_H__
#define __ENEMY_BADGER_H__

#include "Enemy.h"

struct Badger : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
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

	struct MyData : StoredEnemyData
	{
		int attackFrame;
		int landedAction;
		int nextAction;
	};
	MyData data;

	Tileset *ts;

	V2d gravity;
	
	int attackMult;

	double maxGroundSpeed;
	double maxFallSpeed;

	double runAccel;
	double runDecel;

	double jumpStrength;


	Badger(ActorParams *ap);
	void ResetEnemy();
	void SetLevel(int lev);
	void ActionEnded();
	void ProcessState();
	void UpdateEnemyPhysics();
	void HandleNoHealth();
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

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};


#endif