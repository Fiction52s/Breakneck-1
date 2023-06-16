#ifndef __ENEMY_CHEETAH_H__
#define __ENEMY_CHEETAH_H__

#include "Enemy.h"

struct Cheetah : Enemy, GroundMoverHandler
{
	enum Action
	{
		IDLE,
		RUN,
		CHARGE,
		BOOST,
		A_Count
	};

	struct MyData : StoredEnemyData
	{
		int preChargeFrame;
		int chargeFrame;
	};
	MyData data;
	
	int preChargeLimit;
	double boostSpeed;

	double runAccel;
	double runDecel;
	double turnaroundDist;
	double boostPastDist;

	Action landedAction;
	Action nextAction;

	Tileset *ts;

	V2d tempVel;
	V2d gravity;
	
	int chargeLimit;

	int attackFrame;
	int attackMult;

	double maxGroundSpeed;
	double maxFallSpeed;

	double jumpStrength;

	Cheetah(ActorParams *ap);
	void RunMovement();
	void ActionEnded();
	void ProcessState();
	void SetLevel(int lev);
	void UpdateEnemyPhysics();
	//void HandleNoHealth();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void Jump(double strengthx,
		double strengthy);
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