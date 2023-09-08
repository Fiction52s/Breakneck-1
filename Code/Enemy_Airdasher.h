#ifndef __ENEMY_AIRDASHER_H__
#define __ENEMY_AIRDASHER_H__

#include "Enemy.h"

struct Airdasher : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_CHARGE,
		S_DASH,
		S_OUT,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		int hitFrame;
		int currHits;
		V2d currOrig;
		int physStepIndex;
		V2d dashDir;
		int chargeFrames;
		V2d playerDir;
	};
	MyData data;

	CubicBezier dashBez;
	CubicBezier returnBez;
	int maxCharge;
	double speed;
	int hitLimit;
	int dashFrames;
	int returnFrames;
	double dashRadius;

	Tileset *ts;

	Airdasher(ActorParams *ap);
	~Airdasher();
	void ProcessState();

	void SetLevel(int lev);
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void UpdateSprite();
	void ResetEnemy();
	double SetFacingPlayerAngle();
	void SetFacingSide( V2d pDir );
	void IHitPlayer(int index = 0);
	void UpdateHitboxes();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif