#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

//#include "Actor.h"
#include <SFML\Graphics.hpp>

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
		CUBIC,
		RADIAL,
		LINE,
		Count
	};

	Movement( CubicBezier &bez, int duration,
		Types moveType );
	virtual ~Movement();
	void InitDebugDraw();
	CubicBezier bez;
	virtual sf::Vector2<double> GetPosition( int t ) = 0;
	int duration;
	void DebugDraw( sf::RenderTarget *target );
	Movement *next;
	sf::Vertex *vertices;//debugdraw
	sf::Vector2<double> start;
	sf::Vector2<double> end;
	Types moveType;

	//Launcher *launcher;
};

struct WaitMovement : Movement
{
	WaitMovement( sf::Vector2<double> &pos,
		int duration );
	sf::Vector2<double> GetPosition( int t );
	sf::Vector2<double> pos;
};

struct CubicMovement : Movement
{
	CubicMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B,
		sf::Vector2<double> &C,
		sf::Vector2<double> &D,
		CubicBezier &bez,
		int duration );
	 
	sf::Vector2<double> A;
	sf::Vector2<double> B;
	sf::Vector2<double> C;
	sf::Vector2<double> D;

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
	void AddLineMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, CubicBezier&, int duration );
	void AddCubicMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, sf::Vector2<double> &C,
		sf::Vector2<double> &D, CubicBezier&, int duration );
	void AddRadialMovement( sf::Vector2<double> &base, 
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
	void Update( int slowMultiple = 1 );
	void Reset();
	Projectile *projectileList;
	Projectile *currProjectile;
	int currProjectileStartTime;
};

#endif 