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

	struct MovementData
	{
		V2d start;
		V2d end;
		int duration;
		CubicBezier bez;
	};

	V2d start;
	V2d end;
	int duration;
	CubicBezier bez;


	Movement *next;
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
	void StoreMovementData(MovementData &md);
	void SetMovementData(MovementData &md);

	virtual int GetNumStoredBytes() { return 0; }
	virtual void StoreBytes(unsigned char *bytes){ }
	virtual void SetFromBytes(unsigned char *bytes) {}
};

struct WaitMovement : Movement
{
	struct MyData : MovementData
	{
		V2d pos; //probably useless
	};
	MyData data;

	WaitMovement( int duration );
	V2d GetPosition( int t );

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct QuadraticMovement : Movement
{
	struct MyData : MovementData
	{
		V2d A;
		V2d B;
		V2d C;
	};
	MyData data;

	QuadraticMovement(V2d &A,V2d &B,
		V2d &C, CubicBezier &bez,
		int duration);
	double GetArcLength();
	void SetDebugControlPoints();
	V2d GetPosition(int t);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct CubicMovement : Movement
{
	struct MyData : MovementData
	{
		V2d A;
		V2d B;
		V2d C;
		V2d D;
	};

	MyData data;

	CubicMovement( V2d &A,
		V2d &B,
		V2d &C,
		V2d &D,
		CubicBezier &bez,
		int duration );
	void SetDebugControlPoints();
	V2d GetPosition( int t );

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct RadialMovement : Movement
{
	struct MyData : MovementData
	{
		bool clockwise;
		double startAngle;
		double endAngle;
		double radius;
		V2d basePos;
	};

	MyData data;
	
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
	V2d GetPosition( int t );

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct LineMovement: Movement
{
	struct MyData : MovementData
	{
	};

	MyData data;

	LineMovement( V2d &A,
		V2d &B,
		CubicBezier &bez,
		int duration );

	V2d GetPosition( int t );

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

struct GameSession;
struct MovementSequence
{
	struct MyData
	{
		V2d position;
		int currTime;
		Movement *currMovement;
		int currMovementStartTime;
	};
	MyData data;

	Movement *movementList;
	int numTotalMovements;

	MovementSequence();
	~MovementSequence();

	V2d GetPos();

	bool IsMovementActive();
	void AddMovement( Movement *movement );
	void InitMovementDebug();
	void Update(int slowMultiple = 1, int stepsAtOnce = 1);
	void Reset();
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

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif 