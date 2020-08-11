#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

#include <SFML\Graphics.hpp>
#include "VectorMath.h"
#include "CircleGroup.h"

sf::Vector2<double> GetQuadraticValue( 
	sf::Vector2<double> &p0,
	sf::Vector2<double> &p1,
	sf::Vector2<double> &p2,
	sf::Vector2<double> &p3,
	double time );

sf::Vector2<double> GetCubicValue( 
	sf::Vector2<double> &p0,
	sf::Vector2<double> &p1,
	sf::Vector2<double> &p2,
	sf::Vector2<double> &p3,
	double time );

sf::Vector2<double> GetLinearValue( 
	sf::Vector2<double> &p0,
	sf::Vector2<double> &p1,
	sf::Vector2<double> &p2,
	sf::Vector2<double> &p3,
	double time );

struct CubicBezier
{
	CubicBezier()
		:p0( sf::Vector2<double>( 0, 0 ) )
		,p1(sf::Vector2<double>(0, 0))
		, p2(sf::Vector2<double>(1, 1))
		,p3(sf::Vector2<double>(1, 1))

	{}
	CubicBezier( double p1x,
		double p1y,
		double p2x,
		double p2y );


	sf::Vector2<double> p0;
	sf::Vector2<double> p1;
	sf::Vector2<double> p2;
	sf::Vector2<double> p3;

	double GetValue( double a );
	double GetX( double t );
	double GetY( double t );
	//double GetValue( double time );
};

struct Projectile
{
	Projectile();
	int duration;
	Projectile *next;
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
	sf::Vector2<double> start;
	sf::Vector2<double> end;
	Types moveType;
	CircleGroup *circles;
	CircleGroup *controlPointCircles;

	Movement( CubicBezier &bez, int duration,
		Types moveType );
	virtual ~Movement();
	void SetFrameDuration(int f);
	void InitDebugDraw();
	virtual void SetDebugControlPoints() {}
	V2d GetEndVelocity();
	V2d GetFrameVelocity( int f );
	virtual V2d GetPosition( int t ) = 0;
	void DebugDraw( sf::RenderTarget *target );
};

struct WaitMovement : Movement
{
	sf::Vector2<double> pos;

	WaitMovement( int duration );
	sf::Vector2<double> GetPosition( int t );
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
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D;

	CubicMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B,
		sf::Vector2<double> &C,
		sf::Vector2<double> &D,
		CubicBezier &bez,
		int duration );
	void SetDebugControlPoints();
	sf::Vector2<double> GetPosition( int t );
};

struct RadialMovement : Movement
{
	RadialMovement( sf::Vector2<double> &base, 
		double radius, 
		double startAngle, 
		double endAngle, 
		bool clockwise,
		sf::Vector2<double> scale,
		double ellipseAngle,
		CubicBezier &bez,
		int duration );
	bool clockwise;
	double startAngle;
	double endAngle;
	double radius;
	sf::Vector2<double> scale;
	double ellipseAngle;
	sf::Vector2<double> basePos;
	//sf::Transform transform;
	sf::Vector2<double> GetPosition( int t );
};

struct LineMovement: Movement
{
	LineMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B,
		CubicBezier &bez,
		int duration );
		
	//sf::Vector2<double> A;
	//sf::Vector2<double> B;

	sf::Vector2<double> GetPosition( int t );
};

struct GameSession;
struct MovementSequence
{
	MovementSequence();
	~MovementSequence();
	GameSession *owner;
	sf::Vector2<double> position;
	double rotation;
	int currTime;
	void AddMovement( Movement *movement );
	void AddRotation( Rotation *rotation );
	void InitMovementDebug();
	void MovementDebugDraw( sf::RenderTarget *target );
	LineMovement * AddLineMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, CubicBezier&, int duration );
	QuadraticMovement * AddQuadraticMovement(
		V2d &A, V2d &B, V2d &C, CubicBezier &cb,
		int duration);
	CubicMovement * AddCubicMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, sf::Vector2<double> &C,
		sf::Vector2<double> &D, CubicBezier&, int duration );
	RadialMovement * AddRadialMovement( sf::Vector2<double> &base, 
		double radius, double startAngle,
		double endAngle, bool clockwise, 
		sf::Vector2<double> scale,
		double ellipseAngle,
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
	sf::Vector2<double> &GetPosition();
	double GetRotation();
	void Update( int slowMultiple = 1, int stepsAtOnce = 1);
	void Reset();
	Projectile *projectileList;
	Projectile *currProjectile;
	int currProjectileStartTime;
};

#endif 