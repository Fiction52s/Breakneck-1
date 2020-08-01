#ifndef __ENEMY_MOVEMENTTESTER_H__
#define __ENEMY_MOVEMENTTESTER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"

struct MovementTester : Enemy
{
	enum Action
	{
		MOVE,
		WAIT,
		A_Count
	};


	int moveFrames;
	int waitFrames;
	int maxWaitFrames;
	sf::CircleShape predictCircle;

	bool predict;

	int targetPlayerIndex;

	LineMovement *move;
	MovementSequence ms;

	HitboxInfo hitboxInfos[A_Count];

	V2d targetPos;

	MovementTester(ActorParams *ap);

	sf::FloatRect GetAABB();

	void UpdatePreFrameCalculations();
	void ProcessState();
	void CalcTargetAfterHit();
	void UpdateHitboxes();

	void EnemyDraw(sf::RenderTarget *target);


	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void FrameIncrement();

	void SetHitboxInfo(int a);

	sf::CircleShape myCircle;


};

#endif