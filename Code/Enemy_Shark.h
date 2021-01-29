#ifndef __ENEMY_SHARK_H__
#define __ENEMY_SHARK_H__

#include "Enemy.h"

struct Shark : Enemy
{
	enum Action
	{
		NEUTRAL,
		WAKEUP,
		APPROACH,
		CIRCLE,
		FINALCIRCLE,
		RUSH,
		A_Count
	};

	int circleCounter;

	int wakeCounter;
	int wakeCap;

	int circleFrames;
	double attackAngle;

	double latchStartAngle;
	MovementSequence circleSeq;
	MovementSequence rushSeq;

	V2d basePos;
	sf::Color testColor; //for temp anim
	double acceleration;
	double speed;
	V2d origOffset;
	V2d attackOffset;

	Tileset *ts_circle;
	Tileset *ts_bite;
	Tileset *ts_death;

	bool facingRight;

	CubicBezier approachAccelBez;

	V2d offsetPlayer;
	bool latchedOn;

	Shark( ActorParams *ap );
	void SetLevel(int lev);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	V2d GetCircleOffset();
};


#endif