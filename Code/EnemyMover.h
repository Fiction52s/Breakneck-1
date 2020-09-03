#ifndef __ENEMYMOVER_H__
#define __ENEMYMOVER_H__

#include "Movement.h"
#include "PositionInfo.h"

struct Session;
struct PoiInfo;




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
		WAIT,
	};

	sf::Vertex swingQuad[4];

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

	PoiInfo *targetPI;
	V2d targetPos;

	PositionInfo currPosInfo;
	Session *sess;

	double grindSpeed;

	double wireLength;
	V2d swingAnchor;

	bool predict;

	EnemyMover();
	~EnemyMover();
	void Reset();

	void UpdateSwingDebugDraw();
	void InitNodeDebugDraw(int fightType,
		const std::string &str,
		sf::Color c);
	void SetModeWait(int frames);
	void SetModeSwing(
		V2d &swingAnchor,
		double wireLength,
		int frames);
	void SetModeFall(
		double grav,
		int frames);
	void SetModeGrind(
		double speed, int frames);
	void SetModeNodeProjectile(
		V2d &nodePos, V2d &grav, double height);
	void SetModeNodeProjectile(
		PoiInfo *pi, V2d &grav, double height);
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
	void SetModeNodeJump(
		V2d &nodePos,
		double extraHeight);
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