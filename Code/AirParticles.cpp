#include "AirParticles.h"
#include <iostream>
#include "VectorMath.h"
#include "Actor.h"

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


AirParticleEffect::AirParticleEffect( V2d &pos )
	:emitFrame( 0 ), emitDuration( 30 ), particleRate( 1 ), position( pos ), particleAcc( 0 )
{
	dir = V2d( 0, 0 );
	numParticles = emitDuration * particleRate;
	durationToLive = new double[numParticles];
	velocities = new V2d[numParticles];
	positions = new V2d[numParticles];
	int numPoints = numParticles * 4;
	particles = new VertexArray( sf::Quads, numPoints );
	pastParts = 0;
	maxDurationToLive = 45;

	//testing
	sf::VertexArray &va = *particles;
	

	for( int i = 0; i < numPoints; ++i )
	{
		va[i].color = COLOR_TEAL;//Color::Red;
	}

	particleSize = IntRect( 0, 0, 8, 8 );

	for( int i = 0; i < numParticles; ++i )
	{
		ResetParticle( i );
	}

	for( int i = 0; i < numParticles; ++i )
	{
		durationToLive[i] = 0;
	}
}

void AirParticleEffect::SetDirection( V2d &direction )
{
	dir = direction;
}

void AirParticleEffect::KillParticle( int index )
{
	sf::VertexArray &va = *particles;
	va[index * 4+0].position = Vector2f( 0, 0 );
	va[index * 4+1].position = Vector2f( 0, 0 );
	va[index * 4+2].position = Vector2f( 0, 0 );
	va[index * 4+3].position = Vector2f( 0, 0 );
}

void AirParticleEffect::ResetParticle( int index )
{
	int i = index;
	sf::VertexArray &va = *particles;
	durationToLive[i] = maxDurationToLive;

	Transform t;
	int r = ( rand() % 30 ) - 15;
	t.rotate( r );

	V2d d = dir;
	Vector2f rotated = t.transformPoint( Vector2f( d.x, d.y ) );
	 d = V2d( rotated.x, rotated.y );
	
	velocities[i] = d * 4.0;
	positions[i] = position;

	va[i*4+0].position = Vector2f( position.x, position.y ) 
		+ Vector2f( particleSize.left, particleSize.top );
	va[i*4+1].position = Vector2f( position.x, position.y ) 
		+ Vector2f( particleSize.left + particleSize.width, particleSize.top );
	va[i*4+2].position = Vector2f( position.x, position.y ) 
		+ Vector2f( particleSize.left + particleSize.width , particleSize.top + particleSize.height );
	va[i*4+3].position = Vector2f( position.x, position.y ) 
		+ Vector2f( particleSize.left , particleSize.top + particleSize.height  );
}

void AirParticleEffect::Reset()
{
	pastParts = 0;
	particleAcc = 0;
	emitFrame = 0;
	for( int i = 0; i < numParticles; ++i )
	{
		ResetParticle( i );
		durationToLive[i] = 0;
	}
}

void AirParticleEffect::UpdateParticle( int index )
{
	int i = index;
	sf::VertexArray &va = *particles;
	va[i*4+0].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left, particleSize.top );
	va[i*4+1].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width, particleSize.top );
	va[i*4+2].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width , particleSize.top + particleSize.height );
	va[i*4+3].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left , particleSize.top + particleSize.height  );
}

void AirParticleEffect::Update( V2d &playerPos )
{
	int totalParts = particleAcc;
	if( emitFrame < emitDuration )
	{
		if( pastParts < numParticles - 1 )
		{
		
			if( totalParts >= numParticles - 1 )
			{
				totalParts = numParticles - 1;
			}
		
			for( int i = pastParts; i <= totalParts; ++i )
			{
				//cout << "creating particle " << i << endl;
				ResetParticle( i );
			}


			pastParts = totalParts;
			particleAcc += particleRate;
		}
		emitFrame++;
	}

	V2d playerDir;
	VertexArray &va = *particles;
	int maxA = 200;
	for( int i = 0; i < totalParts; ++i )
	{
		if( durationToLive[i] > 0 )
		{
			playerDir = normalize( playerPos - positions[i] );
			positions[i] += velocities[i];
			velocities[i] = normalize ( velocities[i] + playerDir * .4 ) * length( velocities[i] );

			if( durationToLive[i] < maxDurationToLive / 2.0 )
			{
				va[i*4+0].color.a = i / 10.0 * maxA;
				va[i*4+1].color.a = i / 10.0 * maxA;
				va[i*4+2].color.a = i / 10.0 * maxA;
				va[i*4+3].color.a = i / 10.0 * maxA;
			}
			else
			{

				va[i*4+0].color.a = maxA;
				va[i*4+1].color.a = maxA;
				va[i*4+2].color.a = maxA;
				va[i*4+3].color.a = maxA;
			}
			

			durationToLive[i]--;
			UpdateParticle(i);
		}
		else
		{
			KillParticle( i );
		}
	}
}

AirParticleEffect::~AirParticleEffect()
{
	delete [] durationToLive;
	delete [] velocities;
	delete [] positions;
	delete particles;
}

RotaryParticleEffect::RotaryParticleEffect( Actor *pl )
	:emitFrame( 0 ), particleRate( 1 ), particleAcc( 0 ), player( pl )
{
	position = pl->position;
	dir = V2d( 0, 0 );
	angularVel = 1;
	radius = 8;
	angle = 0; //just for now?
	maxDurationToLive = 15;
	numParticles = maxDurationToLive * particleRate; //continuous //emitDuration * particleRate;
	durationToLive = new double[numParticles];
	velocities = new V2d[numParticles];
	positions = new V2d[numParticles];
	int numPoints = numParticles * 4;
	//cout << "creating rotary with 
	particles = new VertexArray( sf::Quads, numPoints );
	pastParts = 0;

	//testing
	sf::VertexArray &va = *particles;
	

	for( int i = 0; i < numPoints; ++i )
	{
		va[i].color = COLOR_TEAL;//Color::Red;
	}

	particleSize = IntRect( 0, 0, 8, 8 );

	for( int i = 0; i < numParticles; ++i )
	{
		ResetParticle( i );
	}

	for( int i = 0; i < numParticles; ++i )
	{
		durationToLive[i] = 0;
	}
	//cout << "constructor" << endl;
}

void RotaryParticleEffect::SetRadius( int r )
{
	radius = r;
}

void RotaryParticleEffect::SetDirection( V2d &direction )
{
	dir = direction;
}

void RotaryParticleEffect::KillParticle( int index )
{
	sf::VertexArray &va = *particles;
	va[index * 4+0].position = Vector2f( 0, 0 );
	va[index * 4+1].position = Vector2f( 0, 0 );
	va[index * 4+2].position = Vector2f( 0, 0 );
	va[index * 4+3].position = Vector2f( 0, 0 );
}

void RotaryParticleEffect::ResetParticle( int index )
{
	//cout << "resetting particle" << endl;
	position = player->position;
	int i = index;
	sf::VertexArray &va = *particles;
	durationToLive[i] = maxDurationToLive;

	Transform t;
	int r = ( rand() % 4 ) - 2;
	t.rotate( r );

	V2d d = dir;
	Vector2f rotated = t.transformPoint( Vector2f( d.x, d.y ) );
	 d = V2d( rotated.x, rotated.y );
	
	velocities[i] = d * 4.0;
	velocities[i] = V2d( 0, 0 ); //d * .5;

	


	//Vector2f adir( cos( angle ), sin(angle ) );
	//cout << "rot: " << player->rotaryAngle << endl;

	
	//Vector2f dirRotNorm( sin( angle ), -cos(angle) );
	V2d velDir = normalize(player->velocity);
	if( player->ground != NULL )
	{
		velDir = normalize(normalize( player->ground->v1 - player->ground->v0 ) * player->groundSpeed);
		if( player->groundSpeed == 0 )
		{
			velDir = normalize( player->ground->v1 - player->ground->v0 ) * 1.0;
		}
	}
	Vector2f vd( velDir.x, velDir.y );
	double f = atan2( velDir.y, velDir.x );

	Vector2f adir( 0, sin( angle ) );
	Transform at;
	at.rotate( f / PI * 180.0 );//player->rotaryAngle / PI * 180 );//player->sprite->getRotation() );
	adir = at.transformPoint( adir );
	//at.transformPoint( adir );
	//Vector2f blah = at.transformPoint( adir );
	V2d adirD( adir.x, adir.y );
	adirD *= radius;//10.0;


	//cout << "ang: " << angularVel << ", radius: " << radius << ", adirD: " << adirD.x << ", " << adirD.y << endl;
	positions[i] = position + adirD;

	angle += angularVel / radius;
	if( angle > PI * 2.0 )
	{
		angle -= PI * 2.0;
	}

	va[i*4+0].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left, particleSize.top );
	va[i*4+1].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width, particleSize.top );
	va[i*4+2].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width , particleSize.top + particleSize.height );
	va[i*4+3].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left , particleSize.top + particleSize.height  );
}

void RotaryParticleEffect::Reset()
{
	pastParts = 0;
	particleAcc = 0;
	emitFrame = 0;
	for( int i = 0; i < numParticles; ++i )
	{
		ResetParticle( i );
		durationToLive[i] = 0;
	}
}

void RotaryParticleEffect::UpdateParticle( int index )
{
	int i = index;
	sf::VertexArray &va = *particles;
	va[i*4+0].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left, particleSize.top );
	va[i*4+1].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width, particleSize.top );
	va[i*4+2].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width , particleSize.top + particleSize.height );
	va[i*4+3].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left , particleSize.top + particleSize.height  );
}

void RotaryParticleEffect::Update( V2d &playerPos )
{
	position = player->position;
	int totalParts = emitFrame;

	ResetParticle( emitFrame );

	emitFrame++;
	if( emitFrame == numParticles )
	{
		emitFrame = 0;
		pastParts = 0;
		totalParts = 0;
		particleAcc = 0;
	}

	//if( emitFrame <  numParticles )
	//{
	
		/*if( pastParts < numParticles - 1 )
		{
		
			if( totalParts >= numParticles - 1 )
			{
				totalParts = numParticles - 1;
			}
			

			if( pastParts == 0 )
			{
				ResetParticle( 0 );
			}
			else
			{
				for( int i = pastParts + 1; i <= totalParts; ++i )
				{
					cout << "creating particle " << i << endl;
					ResetParticle( i );
				}
			}


			pastParts = totalParts;
			particleAcc += particleRate;
		}*/
	
	//}

	V2d playerDir;
	VertexArray &va = *particles;
	int maxA = 200;
	for( int i = 0; i < totalParts; ++i )
	{
		if( durationToLive[i] > 0 )
		{
			playerDir = normalize( playerPos - positions[i] );
			positions[i] += velocities[i];
			//velocities[i] = normalize ( velocities[i] + playerDir * .4 ) * length( velocities[i] );

			if( durationToLive[i] < maxDurationToLive / 2.0 )
			{
				va[i*4+0].color.a = i / (maxDurationToLive / 2.0) * maxA;
				va[i*4+1].color.a = i / (maxDurationToLive / 2.0) * maxA;
				va[i*4+2].color.a = i / (maxDurationToLive / 2.0) * maxA;
				va[i*4+3].color.a = i / (maxDurationToLive / 2.0) * maxA;
			}
			else
			{

				va[i*4+0].color.a = maxA;
				va[i*4+1].color.a = maxA;
				va[i*4+2].color.a = maxA;
				va[i*4+3].color.a = maxA;
			}
			

			durationToLive[i]--;
			UpdateParticle(i);
		}
		else
		{
			KillParticle( i );
		}
	}

	
}

RotaryParticleEffect::~RotaryParticleEffect()
{
	delete [] durationToLive;
	delete [] velocities;
	delete [] positions;
	delete particles;
}

ParticleTrail::ParticleTrail( Actor *pl )
	:emitFrame( 0 ), particleRate( 1 ), particleAcc( 0 ), player( pl )
{
	position = pl->position;
	dir = V2d( 0, 0 );
	angularVel = 1;
	radius = 8;
	angle = 0; //just for now?
	maxDurationToLive = 120;
	numParticles = maxDurationToLive * particleRate; //continuous //emitDuration * particleRate;
	durationToLive = new double[numParticles];
	velocities = new V2d[numParticles];
	positions = new V2d[numParticles];
	int numPoints = numParticles * 4;
	//cout << "creating rotary with 
	particles = new VertexArray( sf::Quads, numPoints );
	pastParts = 0;

	//testing
	sf::VertexArray &va = *particles;
	

	for( int i = 0; i < numPoints; ++i )
	{
		va[i].color = COLOR_TEAL;//Color::Red;
	}

	particleSize = IntRect( 0, 0, 8, 8 );

	for( int i = 0; i < numParticles; ++i )
	{
		ResetParticle( i );
	}

	for( int i = 0; i < numParticles; ++i )
	{
		durationToLive[i] = 0;
	}
	//cout << "constructor" << endl;
}

void ParticleTrail::SetRadius( int r )
{
	radius = r;
}

void ParticleTrail::SetDirection( V2d &direction )
{
	dir = direction;
}

void ParticleTrail::KillParticle( int index )
{
	sf::VertexArray &va = *particles;
	va[index * 4+0].position = Vector2f( 0, 0 );
	va[index * 4+1].position = Vector2f( 0, 0 );
	va[index * 4+2].position = Vector2f( 0, 0 );
	va[index * 4+3].position = Vector2f( 0, 0 );
}

void ParticleTrail::ResetParticle( int index )
{
	//cout << "resetting particle" << endl;
	position = player->position;
	int i = index;
	sf::VertexArray &va = *particles;
	durationToLive[i] = maxDurationToLive;

	/*Transform t;
	int r = ( rand() % 4 ) - 2;
	t.rotate( r );

	V2d d = dir;
	Vector2f rotated = t.transformPoint( Vector2f( d.x, d.y ) );
	 d = V2d( rotated.x, rotated.y );*/
	
	//velocities[i] = d * 4.0;
	velocities[i] = V2d( 0, 0 ); //d * .5;

	


	//Vector2f adir( cos( angle ), sin(angle ) );
	//cout << "rot: " << player->rotaryAngle << endl;

	
	//Vector2f dirRotNorm( sin( angle ), -cos(angle) );
	V2d velDir = player->velocity;
	if( player->ground != NULL )
	{
		velDir = normalize( player->ground->v1 - player->ground->v0 ) * player->groundSpeed;
		if( player->groundSpeed == 0 )
		{
			//velDir = normalize( player->ground->v1 - player->ground->v0 ) * 1.0;
		}
	}
	Vector2f vd( velDir.x, velDir.y );
	double f = atan2( velDir.y, velDir.x );

	//Vector2f adir( 0, sin( angle ) );
	//Transform at;
	//at.rotate( f / PI * 180.0 );//player->rotaryAngle / PI * 180 );//player->sprite->getRotation() );
	//adir = at.transformPoint( adir );
	//V2d adirD( adir.x, adir.y );
	//adirD *= radius;//10.0;


	//cout << "ang: " << angularVel << ", radius: " << radius << ", adirD: " << adirD.x << ", " << adirD.y << endl;
	positions[i] = position ;//+ adirD;
	velocities[i] = velDir;

	va[i*4+0].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left, particleSize.top );
	va[i*4+1].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width, particleSize.top );
	va[i*4+2].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width , particleSize.top + particleSize.height );
	va[i*4+3].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left , particleSize.top + particleSize.height  );
}

void ParticleTrail::Reset()
{
	pastParts = 0;
	particleAcc = 0;
	emitFrame = 0;
	for( int i = 0; i < numParticles; ++i )
	{
		ResetParticle( i );
		durationToLive[i] = 0;
	}
}

void ParticleTrail::UpdateParticle( int index )
{
	int i = index;
	sf::VertexArray &va = *particles;
	va[i*4+0].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left, particleSize.top );
	va[i*4+1].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width, particleSize.top );
	va[i*4+2].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left + particleSize.width , particleSize.top + particleSize.height );
	va[i*4+3].position = Vector2f( positions[i].x, positions[i].y ) 
		+ Vector2f( particleSize.left , particleSize.top + particleSize.height  );
}

void ParticleTrail::Update( V2d &playerPos )
{
	position = player->position;
	int totalParts = emitFrame;

	ResetParticle( emitFrame );

	emitFrame++;
	if( emitFrame == numParticles )
	{
		emitFrame = 0;
		pastParts = 0;
		totalParts = 0;
		particleAcc = 0;
	}

	//if( emitFrame <  numParticles )
	//{
	
		/*if( pastParts < numParticles - 1 )
		{
		
			if( totalParts >= numParticles - 1 )
			{
				totalParts = numParticles - 1;
			}
			

			if( pastParts == 0 )
			{
				ResetParticle( 0 );
			}
			else
			{
				for( int i = pastParts + 1; i <= totalParts; ++i )
				{
					cout << "creating particle " << i << endl;
					ResetParticle( i );
				}
			}


			pastParts = totalParts;
			particleAcc += particleRate;
		}*/
	
	//}

	V2d playerDir;
	VertexArray &va = *particles;
	int maxA = 255;
	for( int i = 0; i < totalParts; ++i )
	{
		if( durationToLive[i] > 0 )
		{
			playerDir = normalize( playerPos - positions[i] );
			positions[i] += velocities[i] / 10.0;
			//velocities[i] = normalize ( velocities[i] + playerDir * .4 ) * length( velocities[i] );

			if( durationToLive[i] < maxDurationToLive / 2.0 )
			{
				va[i*4+0].color.a = i / (maxDurationToLive / 2.0) * maxA;
				va[i*4+1].color.a = i / (maxDurationToLive / 2.0) * maxA;
				va[i*4+2].color.a = i / (maxDurationToLive / 2.0) * maxA;
				va[i*4+3].color.a = i / (maxDurationToLive / 2.0) * maxA;
			}
			else
			{

				va[i*4+0].color.a = maxA;
				va[i*4+1].color.a = maxA;
				va[i*4+2].color.a = maxA;
				va[i*4+3].color.a = maxA;
			}
			

			durationToLive[i]--;
			UpdateParticle(i);
		}
		else
		{
			KillParticle( i );
		}
	}

	
}

ParticleTrail::~ParticleTrail()
{
	delete [] durationToLive;
	delete [] velocities;
	delete [] positions;
	delete particles;
}

