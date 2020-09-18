#include "Movement.h"
#include "Enemy.h"
#include <iostream>
#include <assert.h>

#define TIMESTEP (1.0 / 60.0)

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
	//assert( a < 1.1 );

	if( a > 1.0 )
		a = 1.0;
	//assert( a <= 1.0 );
	//cout << "getting value" << endl;
	double t = .5;
	double step = .5;
	double precision = .0001; //precision to 1/10000th 

	//can reduce the precision later to save time if I want. This is fine for now
	//.001 ~ 10 iterations, .0001 ~ 13 iterations

	double x = GetX( t );
	
	int maxSteps = 20;
	int currStep = 0;
	//int testSteps = 0;
	while( abs( x - a ) > precision )
	{
		//cout << "BLAH VALUE: " << x << ", a: " << a << endl;
		step /= 2.0;
		t += Sign( a - x ) * step;
		x = GetX( t );

		//testSteps++;
		++currStep;

		if (currStep == maxSteps)
			break;
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

Movement::Movement( CubicBezier &p_bez, int dur, Types type )
	:next( NULL ), duration( dur * NUM_MAX_STEPS * 5 ), circles( NULL ), 
	controlPointCircles( NULL ), bez( p_bez ), moveType( type )

{
}

Movement::~Movement()
{
	if( circles != NULL )
	{
		delete circles;
		
	}
	if (controlPointCircles != NULL)
	{
		delete controlPointCircles;
	}
}

void Movement::SetFrameDuration(int f)
{
	//5 is the current slow factor used by time slow bubbles
	duration = f * NUM_MAX_STEPS * 5;
}

void Movement::InitDebugDraw()
{
	int numCircles = 20;
	double div = 1.0 / numCircles;
	if (circles == NULL)
	{
		circles = new CircleGroup(numCircles, 8, Color::White, 6);
		
		if (moveType == QUADRATIC)
		{
			controlPointCircles = new CircleGroup(1, 8, Color::Green, 6);
			//controlPointCircles->SetPosition
		}
		else if (moveType == CUBIC)
		{
			controlPointCircles = new CircleGroup(2, 32, Color::Green, 6);
		}
	}

	circles->ShowAll();

	if (controlPointCircles != NULL)
	{
		controlPointCircles->ShowAll();
	}
	

	double x = div;

	for(int i = 0; i < numCircles; ++i )
	{
		V2d pos = GetPosition( (int)(x * duration) );
		circles->SetPosition(i, Vector2f(pos));
		x += div;
	}

	SetDebugControlPoints();
}

V2d Movement::GetEndVelocity()
{
	int numFrames = duration / (NUM_MAX_STEPS * 5);

	return GetFrameVelocity(numFrames - 1);
}

V2d Movement::GetFrameVelocity(int f)
{
	assert(f > 0);
	return (GetPosition((f) * NUM_MAX_STEPS * 5)
		- GetPosition((f - 1) * NUM_MAX_STEPS * 5));
}

void Movement::DebugDraw( sf::RenderTarget *target )
{
	if (circles != NULL)
	{
		circles->Draw(target);
	}
	
	if (controlPointCircles != NULL)
	{
		controlPointCircles->Draw(target);
	}
}

LineMovement::LineMovement( sf::Vector2<double> &a,
		sf::Vector2<double> &b,
		CubicBezier &bez,
		int duration )
		:Movement( bez, duration, Types::LINE )//, A( a ), B( b )
{
	start = a;
	end = b;
}

V2d LineMovement::GetPosition( int t )
{
	
	//cout << "Start get position" << endl;
	double v = bez.GetValue( t / (double)duration );
	return start + ( end - start ) * v;
}

QuadraticMovement::QuadraticMovement(sf::Vector2<double> &a,
	sf::Vector2<double> &b, sf::Vector2<double> &c,CubicBezier &bez,
	int duration)
	:Movement(bez, duration, Types::QUADRATIC), A(a), B(b), C(c)
{
	start = a;
	end = c;
}



V2d QuadraticMovement::GetPosition(int t)
{
	double v = bez.GetValue(t / (double)duration);
	double rv = (1 - v);
	return rv * (rv * A + v * B) + v * (rv * B + v * C);
}

double QuadraticMovement::GetArcLength()
{
	V2d a, b;
	a.x = A.x - 2 * B.x + C.x;
	a.y = A.y - 2 * B.y + C.y;
	b.x = 2 * B.x - 2 * A.x;
	b.y = 2 * B.y - 2 * A.y;

	double E = 4 * (a.x*a.x + a.y*a.y);
	double F = 4 * (a.x*b.x + a.y*b.y);
	double G = b.x*b.x + b.y*b.y;

	if (abs(E) < 0.0000001 )
	{
		return length(C - A);
	}

	double Sabc = 2 * sqrt(E + F + G);
	double E_2 = sqrt(E);
	double E_32 = 2 * E*E_2;
	double G_2 = 2 * sqrt(G);
	double FE = F / E_2;

	return (E_32*Sabc +
		E_2*F*(Sabc - G_2) +
		(4 * G*E - F*F)*log((2 * E_2 + FE + Sabc) / (FE + G_2))
		) / (4 * E_32);
}

void QuadraticMovement::SetDebugControlPoints()
{
	controlPointCircles->SetPosition(0, Vector2f(B));
}

CubicMovement::CubicMovement( sf::Vector2<double> &a,
		sf::Vector2<double> &b,
		sf::Vector2<double> &c,
		sf::Vector2<double> &d,
		CubicBezier &bez,
		int duration )
		:Movement( bez, duration, Types::CUBIC ), A( a ), B( b ), C( c), D(d)
{
	start = a;
	end = d;
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

void CubicMovement::SetDebugControlPoints()
{
	controlPointCircles->SetPosition(0, Vector2f(B));
	controlPointCircles->SetPosition(1, Vector2f(C));
	controlPointCircles->SetColor(1, Color::Red);
}

RadialMovement::RadialMovement( V2d &circleBase, V2d &startPos,
		double p_endAngle, 
		bool p_clockwise,
		CubicBezier &bez,
		int duration )
		:Movement( bez, duration, Types::RADIAL )
{
	Set(circleBase, startPos, p_endAngle, p_clockwise,
		bez, duration );
}

void RadialMovement::Set(V2d &base, V2d &startPos, double p_endAngle,
	bool p_clockwise, CubicBezier &p_bez, int frameDuration)
{
	SetFrameDuration(frameDuration);
	basePos = base;
	radius = length(base - startPos);
	startAngle = GetVectorAngleCW(normalize(startPos - base));
	endAngle = p_endAngle;
	clockwise = p_clockwise; 

	if (clockwise)
	{
		if (startAngle > endAngle)
		{
			endAngle += PI * 2;
		}
	}
	else
	{
		if (startAngle < endAngle)
		{
			startAngle += PI * 2;
		}
	}

	start = startPos;
	end = V2d(radius * cos(endAngle), radius * sin(endAngle)) + basePos;//GetPosition(duration);
}

V2d RadialMovement::GetPosition( int t )
{
	double v = bez.GetValue( t / (double)duration );
	double angle = startAngle + ( endAngle - startAngle ) * v;

	return V2d(radius * cos(angle), radius * sin(angle)) + basePos;
}

WaitMovement::WaitMovement( int duration )
	:Movement( CubicBezier(), duration, Types::WAIT )//, pos( p_pos )
{
	//start = pos;
	//end = pos;
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

MovementSequence::~MovementSequence()
{
	Movement *curr = movementList;
	Movement *next = NULL;
	while (curr != NULL)
	{
		next = curr->next;
		delete curr;
		curr = next;
	}
	movementList = NULL;
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

LineMovement * MovementSequence::AddLineMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, CubicBezier& bez, int duration )
{
	LineMovement *lm = new LineMovement(A, B, bez, duration);
	AddMovement( lm );
	return lm;
}

QuadraticMovement * MovementSequence::AddQuadraticMovement(
	V2d &A, V2d &B, V2d &C, CubicBezier &bez,
	int duration)
{
	QuadraticMovement * qm = new QuadraticMovement(A, B, C, bez, duration);
	AddMovement(qm);
	return qm;
}

CubicMovement * MovementSequence::AddCubicMovement( sf::Vector2<double> &A,
		sf::Vector2<double> &B, sf::Vector2<double> &C,
		sf::Vector2<double> &D, CubicBezier& bez, int duration )
{
	CubicMovement *cm = new CubicMovement(A, B, C, D, bez, duration);
 	AddMovement( cm );
	return cm;
}

RadialMovement * MovementSequence::AddRadialMovement( V2d &base, V2d &startPos,
		double endAngle, bool clockwise, CubicBezier &bez, int duration )
{
	RadialMovement *rm = new RadialMovement(base, startPos, endAngle, clockwise,
		bez, duration);
	AddMovement( rm );
	return rm;
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

void MovementSequence::Update( int slowMultiple, int stepsAtOnce )
{
	if( currMovement != NULL )
	{
		//cout << "before" << endl;

		//first one

		if (currMovement->moveType != Movement::WAIT)
		{
			//cout << "localtime: " << (currTime - currMovementStartTime) << ", dur: " << currMovement->duration << endl;
			if (currTime - currMovementStartTime == 0)
			{
				position = currMovement->start;
			}
			//last one
			else if (currTime - currMovementStartTime == currMovement->duration)
			{
				position = currMovement->end;
			}
			else
			{
				position = currMovement->GetPosition(currTime - currMovementStartTime);
			}
		}
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


	currTime += (5 / slowMultiple) * stepsAtOnce;
	//currTime++;
	if( currMovement != NULL )//&& currTime >= currMovement->duration + currMovementStartTime + 1 )
	{
		if ((currMovement->next != NULL && currTime >= currMovement->duration + currMovementStartTime )
			|| (currMovement->next == NULL && currTime >= currMovement->duration + currMovementStartTime + 1 ))
		{
			currMovement = currMovement->next;

			if (currMovement != NULL)
			{
				//cout << "switching:" << currTime << endl;
				currMovementStartTime = currTime;
			}
		}
	}
	if( currRotation != NULL )
	{
		if ((currRotation->next != NULL && currTime >= currRotation->duration + currRotationStartTime)
			|| (currRotation->next == NULL && currTime >= currRotation->duration + currRotationStartTime + 1))
		{
			currRotation = currRotation->next;

			if (currRotation != NULL)
			{
				currRotationStartTime = currTime;
			}
		}
	}
	if( currProjectile != NULL )
	{
		if ((currProjectile->next != NULL && currTime >= currProjectile->duration + currProjectileStartTime)
			|| (currProjectile->next == NULL && currTime >= currProjectile->duration + currProjectileStartTime + 1))
		{
			currProjectile = currProjectile->next;

			if (currProjectile != NULL)
			{
				currProjectileStartTime = currTime;
			}
		}
	}

	/*if( currMovement == NULL && currRotation == NULL )
	{
		return false;
	}

	return true;*/
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