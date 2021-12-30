#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

#include <SFML\Graphics.hpp>
#include "VectorMath.h"
#include "CircleGroup.h"

V2d GetQuadraticValue( 
	V2d &p0,
	V2d &p1,
	V2d &p2,
	V2d &p3,
	double time );

V2d GetCubicValue(
	V2d &p0,
	V2d &p1,
	V2d &p2,
	V2d &p3,
	double time );

V2d GetLinearValue(
	V2d &p0,
	V2d &p1,
	V2d &p2,
	V2d &p3,
	double time );

struct CubicBezier
{
	CubicBezier()
		:p0(V2d( 0, 0 ) )
		,p1(V2d(0, 0))
		, p2(V2d(1, 1))
		,p3(V2d(1, 1))

	{}
	CubicBezier( double p1x,
		double p1y,
		double p2x,
		double p2y );


	V2d p0;
	V2d p1;
	V2d p2;
	V2d p3;

	double GetValue( double a );
	double GetX( double t );
	double GetY( double t );
	//double GetValue( double time );
};

struct Rotation
{
	Rotation();
	virtual double GetRotation(int t ) = 0;
	int duration;
	Rotation *next;
};

struct Movement
{
	enum Types
	{
		WAIT,
		QUADRATIC,
		CUBIC,
		RADIAL,
		LINE,
		Count
	};

	CubicBezier bez;
	int duration;
	Movement *next;
	V2d start;
	V2d end;
	Types moveType;
	CircleGroup *circles;
	CircleGroup *controlPointCircles;

	Movement( CubicBezier &bez, int duration,
		Types moveType );
	virtual ~Movement();
	void SetFrameDuration(int f);
	int GetFrameDuration();
	void InitDebugDraw();
	virtual void SetDebugControlPoints() {}
	V2d GetEndVelocity();
	V2d GetFrameVelocity( int f );
	virtual V2d GetPosition( int t ) = 0;
	void DebugDraw( sf::RenderTarget *target );
};

struct WaitMovement : Movement
{
	V2d pos;

	WaitMovement( int duration );
	V2d GetPosition( int t );
};

struct QuadraticMovement : Movement
{
	V2d A;
	V2d B;
	V2d C;

	QuadraticMovement(V2d &A,V2d &B,
		V2d &C, CubicBezier &bez,
		int duration);
	double GetArcLength();
	void SetDebugControlPoints();
	V2d GetPosition(int t);
};

struct CubicMovement : Movement
{
	V2d A;
	V2d B;
	V2d C;
	V2d D;

	CubicMovement( V2d &A,
		V2d &B,
		V2d &C,
		V2d &D,
		CubicBezier &bez,
		int duration );
	void SetDebugControlPoints();
	V2d GetPosition( int t );
};

struct RadialMovement : Movement
{
	RadialMovement( V2d &base, 
		V2d &startPos,
		double endAngle, 
		bool clockwise,		
		CubicBezier &bez,
		int duration );
	void Set(V2d &base,
		V2d &startPos,
		double endAngle,
		bool clockwise,
		CubicBezier &bez,
		int frameDuration);
	void Set(V2d &base,
		V2d &startPos,
		double endAngle,
		double speed );
	void SetDebugControlPoints();
	bool clockwise;
	double startAngle;
	double endAngle;
	double radius;
	V2d basePos;
	V2d GetPosition( int t );
};

struct LineMovement: Movement
{
	LineMovement( V2d &A,
		V2d &B,
		CubicBezier &bez,
		int duration );
		
	//V2d A;
	//V2d B;

	V2d GetPosition( int t );
};

struct GameSession;
struct MovementSequence
{
	MovementSequence();
	~MovementSequence();
	GameSession *owner;
	V2d position;
	double rotation;
	int currTime;
	void AddMovement( Movement *movement );
	void AddRotation( Rotation *rotation );
	void InitMovementDebug();
	void MovementDebugDraw( sf::RenderTarget *target );
	LineMovement * AddLineMovement( V2d &A,
		V2d &B, CubicBezier&, int duration );
	QuadraticMovement * AddQuadraticMovement(
		V2d &A, V2d &B, V2d &C, CubicBezier &cb,
		int duration);
	CubicMovement * AddCubicMovement( V2d &A,
		V2d &B, V2d &C,
		V2d &D, CubicBezier&, int duration );
	RadialMovement * AddRadialMovement( V2d &base, 
		V2d &startPos, double endAngle, 
		bool clockwise, 
		CubicBezier &bez, int duration );
		

	Movement *movementList;
	Movement *currMovement;
	//bool justChanged;
	
	//^is almost always null
	//but when its fed a projectile it
	//tells u to shoot?
	int currMovementStartTime;
	Rotation *rotationList;
	Rotation *currRotation;
	
	int currRotationStartTime;
	void Update( int slowMultiple = 1, int stepsAtOnce = 1);
	void Reset();
	//Projectile *projectileList;
	//Projectile *currProjectile;
	//int currProjectileStartTime;
};

#endif 