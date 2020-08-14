#ifndef __ENEMY_MOVEMENTTESTER_H__
#define __ENEMY_MOVEMENTTESTER_H__

#include "Enemy.h"
#include "Bullet.h"
#include "Movement.h"
#include "Enemy_BirdShuriken.h"

//struct SpaceMover;

//figure out the comboer later
//when you need to generalize it for other bosses.

//struct PlayerComboer
//{
//	Enemy *myEnemy;
//	V2d targetPos;
//	sf::CircleShape predictCircle;
//	bool predict;
//
//	PlayerComboer(Enemy *e);
//	void PredictNextFrame();
//	void CalcTargetAfterHit(int pIndex);
//	void UpdatePreFrameCalculations( int pIndex );
//};

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
		NODE_DOUBLE_QUADRATIC,
		NODE_PROJECTILE,
	};

	MoveType moveType;

	LineMovement *linearMove;
	QuadraticMovement *quadraticMove;
	CubicMovement *cubicMove;
	QuadraticMovement *doubleQuadtraticMove0;
	QuadraticMovement *doubleQuadtraticMove1;

	MovementSequence linearMovementSeq;
	MovementSequence quadraticMovementSeq;
	MovementSequence cubicMovementSeq;
	MovementSequence doubleQuadraticMovementSeq;

	CircleGroup *nodeCircles;
	CircleGroup *debugCircles;

	V2d *chaseTarget;
	V2d chaseOffset;
	double chaseMaxVel;
	double chaseAccel;
	
	int actionFrames;

	V2d lastActionEndVelocity;

	V2d projectileGrav;
	V2d velocity;


	
	bool predict;

	Enemy *myEnemy;
	
	EnemyMover( Enemy *e );
	~EnemyMover();
	void InitNodeDebugDraw(int fightType,
		const std::string &str, 
		sf::Color c );
	void Reset();
	void SetModeNodeProjectile(
		V2d &nodePos, V2d &grav, double height);
	void SetModeNodeLinear(
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration);
	void SetModeNodeLinearConstantSpeed(
		V2d &nodePos,
		CubicBezier &cb,
		double speed);
	void SetModeNodeQuadratic(
		V2d &controlPoint0,
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration);
	void SetModeNodeQuadraticConstantSpeed(
		V2d &controlPoint0,
		V2d &nodePos,
		CubicBezier &cb,
		double speed);
	void SetModeNodeCubic(
		V2d &controlPoint0,
		V2d &controlPoint1,
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration);
	void SetModeNodeCubicConstantSpeed(
		V2d &controlPoint0,
		V2d &controlPoint1,
		V2d &nodePos,
		CubicBezier &cb,
		double speed);
	void SetModeNodeDoubleQuadratic(
		V2d &controlPoint0,
		V2d &nodePos,
		CubicBezier &cb,
		int frameDuration,
		double spreadFactor );
	void SetModeNodeDoubleQuadraticConstantSpeed(
		V2d &controlPoint0,
		V2d &nodePos,
		CubicBezier &cb,
		double speed,
		double spreadFactor);
	void SetModeChase(V2d *chasePos,
		V2d &chaseOffset, double maxVel,
		double accel,
		int frameDuration = -1);
	void SetModeNodeJump(
		V2d &nodePos,
		double extraHeight);
	void FrameIncrement();
	V2d UpdatePhysics();
	int GetLinearFrameEstimate(double attemptSpeed,
		V2d &start, V2d &end);
	int GetQuadraticFrameEstimate(double attemptSpeed,
		V2d &start, V2d &cp0, V2d &end);
	int GetCubicFrameEstimate(double attemptSpeed,
		V2d &start, V2d &cp0, V2d &cp1, V2d &end);
	int GetDoubleQuadraticFrameEstimate(
		double attemptSpeed,
		V2d &start, V2d &cp0, V2d &end,
		double spreadFactor);
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

	EnemyMover enemyMover;

	int testCounter;

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