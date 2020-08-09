#ifndef __ENEMY_MOVEMENTTESTER_H__
#define __ENEMY_MOVEMENTTESTER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_BirdShuriken.h"

//struct SpaceMover;

struct EnemyMover
{
	enum MoveType
	{
		NONE,
		CHASE,
		APPROACH,
		NODE_LINEAR,
		NODE_QUADRATIC,
		NODE_CUBIC,
	};

	MoveType moveType;

	LineMovement *linearMove;
	QuadraticMovement *quadraticMove;
	CubicMovement *cubicMove;

	MovementSequence linearMovementSeq;
	MovementSequence quadraticMovementSeq;
	MovementSequence cubicMovementSeq;

	

	V2d *chaseTarget;
	V2d chaseOffset;
	double chaseMaxVel;
	double chaseAccel;
	V2d chaseVelocity;

	bool predict;

	Enemy *myEnemy;
	
	EnemyMover( Enemy *e );
	void Reset();
	void SetModeNodeLinear(
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration);
	void SetModeNodeQuadratic(
		V2d &controlPoint0,
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration);
	void SetModeChase(V2d *chasePos,
		V2d &chaseOffset, double maxVel,
		double accel);
	V2d UpdatePhysics();
	void DebugDraw(sf::RenderTarget *target);
};

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
		CHASE,
		APPROACH,
		NODE_LINEAR,
		NODE_QUADRATIC,
		NODE_CUBIC,
		CURVE,
	};
	EnemyMover enemyMover;

	BirdShurikenPool shurPool;

	double approachStartDist;
	CubicBezier approachBez;
	
	V2d *chaseTarget;
	V2d chaseOffset;
	double chaseMaxVel;
	double chaseAccel;

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

	int testCheck;

	HitboxInfo hitboxInfos[A_Count];

	V2d targetPos;

	MovementTester(ActorParams *ap);
	~MovementTester();

	sf::FloatRect GetAABB();

	void SetModeNodeLinear(
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration );
	void SetModeNodeQuadratic(
		V2d &controlPoint0,
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration);
	void SetModeChase(V2d *chasePos,
		V2d &chaseOffset, double maxVel,
		double accel);
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