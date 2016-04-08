#include "Movement.h"
#include "Enemy.h"
#include <iostream>

#define TIMESTEP 1.0 / 60.0
#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

using namespace std;
using namespace sf;

V2d GetCubicValue( V2d &p0,V2d &p1,V2d &p2,V2d &p3,double time )
{
	double rtime = ( 1 - time );
	return pow( rtime, 3 ) * p0 
		+ 3 * rtime * rtime * time * p1 
		+ 3 * rtime * time * time * p2 
		+ pow( time, 3 ) * p3;
}

V2d GetQuadraticValue( V2d &p0,V2d &p1,V2d &p2,V2d &p3,double time )
{
	double rtime = ( 1 - time );
	return rtime * rtime * p0 + 2 * rtime * time * p1 + time * time * p2;
}

V2d GetLinearValue( V2d &p0,V2d &p1,V2d &p2,V2d &p3,double time )
{
	return ( 1- time ) * p0 + time * p1;
}

CubicBezier::CubicBezier( double p1x, double p1y, double p2x, double p2y )
	:p1( p1x, p1y), p2( p2x, p2y )
{
	p0 = V2d( 0, 0 );
	p3 = V2d( 1, 1 );
}

int Sign( double x )
{
	if (x > 0) return 1;
	if (x < 0) return -1;
	return 0;
}

double CubicBezier::GetValue( double a )
{
	//cout << "getting value" << endl;
	double t = .5;
	double step = .5;
	double precision = .0001; //precision to 1/10000th 

	//can reduce the precision later to save time if I want. This is fine for now
	//.001 ~ 10 iterations, .0001 ~ 13 iterations

	double x = GetX( t );
	
	//int testSteps = 0;
	while( abs( x - a ) > precision )
	{
		//cout << "BLAH VALUE: " << x << ", a: " << a << endl;
		step /= 2.0;
		t += Sign( a - x ) * step;
		x = GetX( t );

		//testSteps++;
	}
	//cout << "steps: " << testSteps << endl;

	return GetY( t );
}

double CubicBezier::GetX( double t )
{
	double rt = ( 1 - t);
	return pow( rt, 3 ) * p0.x 
		+ 3 * rt * rt * t * p1.x
		+ 3 * rt * t * t * p2.x
		+ pow( t, 3 ) * p3.x;
}

double CubicBezier::GetY( double t )
{
	double rt = ( 1 - t);
	return pow( rt, 3 ) * p0.y 
		+ 3 * rt * rt * t * p1.y
		+ 3 * rt * t * t * p2.y
		+ pow( t, 3 ) * p3.y;
}

Movement::Movement( CubicBezier &p_bez, int dur )
	:next( NULL ), duration( dur * NUM_STEPS ), vertices( NULL ), bez( p_bez )

{
}

Movement::~Movement()
{
	if( vertices != NULL )
	{
		delete [] vertices;
	}
}

void Movement::InitDebugDraw()
{

	//for debugdraw
	vertices = new Vertex[20 * 4];
	Color col = COLOR_BLUE;
	
	double x = 0;

	int hSize = 4;

	for(int i = 0; i < 20; ++i )
	{
		vertices[i*4+0].color = col;
		vertices[i*4+1].color = col;
		vertices[i*4+2].color = col;
		vertices[i*4+3].color = col;

		V2d pos = GetPosition( (int)(x * duration) );
		vertices[i*4+0].position = Vector2f( pos.x - hSize, pos.y - hSize );
		vertices[i*4+1].position = Vector2f( pos.x + hSize, pos.y - hSize );
		vertices[i*4+2].position = Vector2f( pos.x + hSize, pos.y + hSize );
		vertices[i*4+3].position = Vector2f( pos.x - hSize, pos.y + hSize );
		x += 1 / 20.0;
	}
}

void Movement::DebugDraw( sf::RenderTarget *target )
{
	target->draw( vertices, 80, sf::Quads );	
}

LineMovement::LineMovement( sf::Vector2<double> &a,
		sf::Vector2<double> &b,
		CubicBezier &bez,
		int duration )
		:Movement( bez, duration ), A( a ), B( b )
{
}

V2d LineMovement::GetPosition( int t )
{
	
	//cout << "Start get position" << endl;
	double v = bez.GetValue( t / (double)duration );
	return A + ( B - A ) * v;
}

CubicMovement::CubicMovement( sf::Vector2<double> &a,
		sf::Vector2<double> &b,
		sf::Vector2<double> &c,
		sf::Vector2<double> &d,
		CubicBezier &bez,
		int duration )
		:Movement( bez, duration ), A( a ), B( b ), C( c), D(d)
{
}

V2d CubicMovement::GetPosition( int t )
{
	double v = bez.GetValue( t / (double)duration );
	double rv = ( 1 - v);
	return pow( rv, 3 ) * A
		+ 3 * rv * rv * v * B
		+ 3 * rv * v * v * C
		+ pow( v, 3 ) * D;
}
		
WaitMovement::WaitMovement(  sf::Vector2<double> &p_pos, int duration )
	:Movement( CubicBezier(), duration ), pos( p_pos )
{
}

sf::Vector2<double> WaitMovement::GetPosition( int t )
{
	return pos;
}

MovementSequence::MovementSequence()
	:movementList( NULL ), currMovement( NULL ), rotationList( NULL), 
	currRotation( NULL ), currProjectile( NULL )
{
	Reset();
}

//void MovementSequence::PushMovementLauncher( int numTotalBullets, 
//	int bulletsPerShot, sf::Vector2<double> &position,
//	sf::Vector2<double> &direction, double angleSpread )
//{
//	Movement *curr = movementList;
//	while( curr->next != NULL )
//	{
//		curr = curr->next;
//	}
//	//curr is final now
//	if( curr->launcher != NULL )
//	{
//		delete curr->launcher;
//		cout << "WRITING OVER LAUNCHER!!!" << endl;
//	}
//	else
//	{
//		curr->launcher = new Launcher( owner, numTotalBullets,
//			bulletsPerShot, position, direction, angleSpread );
//	}
//}

void MovementSequence::AddLineMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, CubicBezier& bez, int duration )
{
	AddMovement( new LineMovement( A, B, bez, duration ) );
}

void MovementSequence::AddCubicMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, sf::Vector2<double> &C,
		sf::Vector2<double> &D, CubicBezier& bez, int duration )
{
 	AddMovement( new CubicMovement( A,B,C,D,bez,duration ) );
}

void MovementSequence::InitMovementDebug()
{
	Movement *curr = movementList;
	while( curr != NULL )
	{
		curr->InitDebugDraw();
		curr = curr->next;
	}
}

void MovementSequence::MovementDebugDraw( sf::RenderTarget *target )
{
	if( currMovement != NULL )
	{
		currMovement->DebugDraw( target );
	}
}

void MovementSequence::Reset()
{
	currTime = 0;
	currMovementStartTime = 0;
	currRotationStartTime = 0;

	currMovement = movementList;
	currRotation = rotationList;
}

void MovementSequence::Update()
{
	if( currMovement != NULL )
	{
		//cout << "before" << endl;
		position = currMovement->GetPosition( currTime - currMovementStartTime  );
		//cout << "after" << endl;
		//cout << "updating pos: " << position.x << ", " << position.y << endl;
	}
	if( currRotation != NULL )
	{
		rotation = currRotation->GetRotation( currTime );
	}
	if( currProjectile != NULL )
	{
		//do later
	}



	currTime++;
	if( currMovement != NULL && currTime == currMovement->duration + currMovementStartTime )
	{
		currMovement = currMovement->next;
		//currLauncherList = currMovement->launcher;



		//Launcher *currLaunch = currLauncherList;

		//justChanged = true;
		//while( currLaunch!= NULL )
		//{
		//	currLaunch = currLaunch->next;
		//}

		if( currMovement != NULL )
		{
			currMovementStartTime = currTime;

		}
	}
	if( currRotation != NULL && currTime == currRotation->duration + currRotationStartTime )
	{
		currRotation = currRotation->next;

		if( currRotation != NULL )
		{
			currRotationStartTime = currTime;
		}
	}
	if( currProjectile != NULL && currTime == currProjectile->duration + currProjectileStartTime )
	{
		currProjectile = currProjectile->next;

		if( currProjectile != NULL )
		{
			currProjectileStartTime = currTime;
		}
	}
}

void MovementSequence::AddMovement( Movement *movement )
{
	if( movementList == NULL )
	{
		movementList = movement;
		movement->next = NULL;
	}
	else
	{
		Movement *curr = movementList;
		while( curr->next != NULL )
		{
			curr = curr->next;
		}
		//curr is final now
		curr->next = movement;
		movement->next = NULL;
	}

	Reset();
}

void MovementSequence::AddRotation( Rotation *rotation )
{
	if( rotationList == NULL )
	{
		rotationList = rotation;
		rotation->next = NULL;
	}
	else
	{
		Rotation *curr = rotationList;
		while( curr->next != NULL )
		{
			curr = curr->next;
		}
		//curr is final now
		curr->next = rotation;
		rotation->next = NULL;
	}

	Reset();
}