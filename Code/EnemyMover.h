#ifndef __ENEMYMOVER_H__
#define __ENEMYMOVER_H__

#include "Movement.h"
#include "PositionInfo.h"

struct Session;
struct PoiInfo;

struct EnemyMoverHandler
{
	virtual void HandleFinishTargetedMovement() {}
};


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
		GRIND,
		FALL,
		SWING,
		RADIAL,
		WAIT,
		SWINGJUMP,
		RADIAL_DOUBLE_JUMP,
		ZIP_AND_FALL,
		CATCH,
	};

	sf::Vertex swingQuad[4];

	EnemyMoverHandler * handler;
	MoveType moveType;

	LineMovement *linearMove;
	QuadraticMovement *quadraticMove;
	CubicMovement *cubicMove;
	QuadraticMovement *doubleQuadtraticMove0;
	QuadraticMovement *doubleQuadtraticMove1;
	RadialMovement *radialMove;

	MovementSequence linearMovementSeq;
	MovementSequence quadraticMovementSeq;
	MovementSequence cubicMovementSeq;
	MovementSequence doubleQuadraticMovementSeq;
	MovementSequence radialMovementSeq;

	CircleGroup *nodeCircles;
	CircleGroup *debugCircles;

	V2d *chaseTarget;
	V2d chaseOffset;
	double chaseMaxVel;
	double chaseAccel;
	double catchSpeed;

	int actionFrame;
	int actionTotalDuration;

	V2d lastActionEndVelocity;

	V2d projectileGrav;
	V2d velocity;

	PoiInfo *targetPI;
	V2d targetPos;

	PositionInfo currPosInfo;
	Session *sess;

	double grindSpeed;

	double wireLength;
	V2d swingAnchor;

	bool predict;

	V2d jumpDest; //radial double jump

	//double radialMovementRadius;
	//double radialMovementSpeed;

	EnemyMover();
	~EnemyMover();
	void Reset();
	double GetActionProgress();

	PositionInfo CheckGround(double dist);

	bool IsIdle();
	void FinishTargetedMovement();

	void SetModeZipAndFall(V2d &zipPos,
		V2d &grav, V2d &dest );
	//void SetModeRadialDoubleJump( )
	void SetDestNode(PoiInfo *pi);
	void SetModeRadial( V2d &base,
		double speed, V2d &dest );
	void SetModeRadialDoubleJump(V2d &base,
		double speed, V2d &jumpStart, 
		V2d &dest);
	void UpdateSwingDebugDraw();
	void InitNodeDebugDraw(int fightType,
		const std::string &str,
		sf::Color c);
	void SetModeWait(int frames);
	void SetMoveType(MoveType mt);
	void SetModeSwing(
		V2d &p_swingAnchor, V2d &startPos,
		double endAngle, double startSpeed);
	void SetModeSwingJump(
		V2d &dest,
		V2d &swingAnchor,
		int frames);
	void SetModeFall(
		double grav,
		int frames);
	void SetModeGrind(
		double speed, int frames);
	int SetModeNodeProjectile(
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
	int SetModeNodeQuadraticConstantSpeed(
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
		double spreadFactor);
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
	void SetModeCatch(V2d *chaseTarget,
		double startVel,
		double accel);
	void SetModeApproach(V2d *approachPos,
		V2d &approachOffset, int framesToLock,
		int framesWhileLocked);
	int SetModeNodeJump(
		V2d &nodePos,
		double extraHeight,
		double jumpSpeed = 20);
	void FrameIncrement();
	void UpdatePhysics(int numPhysSteps,
		int slowMultiple);
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

#endif