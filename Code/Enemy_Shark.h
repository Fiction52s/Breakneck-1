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

	struct MyData : StoredEnemyData
	{
		int circleCounter;
		int wakeCounter;
		int circleFrames;
		double latchStartAngle;
		V2d basePos;
		V2d origOffset;
		V2d attackOffset;
		V2d offsetPlayer;
		bool latchedOn;
	};
	MyData data;

	int wakeCap;
	double attackAngle;
	
	MovementSequence circleSeq;
	MovementSequence rushSeq;

	RadialMovement *circleMovement;
	
	sf::Color testColor; //for temp anim
	double acceleration;
	double speed;
	
	Tileset *ts_circle;
	Tileset *ts_bite;
	Tileset *ts_death;

	bool facingRight;

	CubicBezier approachAccelBez;

	Shark( ActorParams *ap );
	void SetLevel(int lev);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	V2d GetCircleOffset();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};


#endif