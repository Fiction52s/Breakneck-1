#ifndef __ENEMY_GHOST_H__
#define __ENEMY_GHOST_H__

#include "Enemy.h"

struct Ghost : Enemy
{
	enum Action
	{
		WAKEUP,
		APPROACH,
		BITE,
		EXPLODE,
		RETURN,
		A_Count
	};


	double detectionRadius;
	double latchStartAngle;

	int awakeFrames;
	int awakeCap;

	bool latchedOn;
	V2d basePos;

	double acceleration;
	double speed;

	int approachFrames;
	int totalFrame;
	V2d origOffset;

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	CubicBezier approachAccelBez;

	V2d offsetPlayer;

	Ghost(ActorParams *ap);
	void SetLevel(int lev);
	void ProcessState();
	void UpdateEnemyPhysics();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void Bite();
};

#endif