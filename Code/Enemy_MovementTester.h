#ifndef __ENEMY_MOVEMENTTESTER_H__
#define __ENEMY_MOVEMENTTESTER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"

//struct SpaceMover;

struct MovementTester : Enemy
{
	enum Action
	{
		MOVE,
		WAIT,
		A_Count
	};


	enum MoveType
	{
		CURVE,
		HOMING,
	};

	V2d velocity;
	double accel;
	double maxSpeed;
	MoveType moveType;
	//SpaceMover *sMover;
	int moveFrames;
	int startMoveFrames;
	V2d startMovePlayerPos;
	int waitFrames;
	int maxWaitFrames;
	sf::CircleShape predictCircle;

	bool predict;

	int targetPlayerIndex;

	LineMovement *move;
	MovementSequence ms;

	MovementSequence curveMovement;
	//CubicMovement *curve;
	QuadraticMovement *qCurve;

	HitboxInfo hitboxInfos[A_Count];

	V2d targetPos;

	MovementTester(ActorParams *ap);
	~MovementTester();

	sf::FloatRect GetAABB();

	void DebugDraw(sf::RenderTarget *target);
	void CalcMovement();
	void UpdatePreFrameCalculations();
	void ProcessState();
	void CalcTargetAfterHit();
	void CalcPlayerFuturePos(int frames);
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