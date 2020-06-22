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
		S_COMBO,
		S_Count
	};

	Airdasher(ActorParams *ap);
	~Airdasher();
	void ProcessState();
	void Setup();

	void SetLevel(int lev);
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	double SetFacingPlayerAngle();
	void SetFacingSide( V2d pDir );
	void ComboHit();
	void IHitPlayer(int index = 0);

	int hitLimit;
	int currHits;

	V2d playerDir;
	V2d velocity;

	int hitFrame;
	int chargeFrames;
	int maxCharge;

	CubicBezier dashBez;
	CubicBezier returnBez;

	double dashRadius;
	int dashFrames;
	int returnFrames;

	int physStepIndex;
	V2d currOrig;
	double speed;

	Tileset *ts;
	Tileset *ts_aura;
};

#endif