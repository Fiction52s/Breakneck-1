#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Spider.h"
#include "EditorTerrain.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Spider::Spider( ActorParams *ap )
	:Enemy( EnemyType::EN_SPIDER, ap )
{
	SetNumActions(A_Count);
	SetEditorActions(MOVE, 0, 0);

	SetLevel(ap->GetLevel());

	gravity = V2d( 0, .6 );
	maxGroundSpeed = 20;

	maxFallSpeed = 25;

	attackFrame = -1;
	attackMult = 10;

	CreateSurfaceMover(startPosInfo, 32, this);
	surfaceMover->SetSpeed( 0 );

	ts = sess->GetSizedTileset( "Enemies/W1/crawler_160x160.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 0;
	hitboxInfo->knockback = 0;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	laserInfo0 = new HitboxInfo;
	laserInfo0->damage = 6;
	laserInfo0->drainX = 0;
	laserInfo0->drainY = 0;
	laserInfo0->hitlagFrames = 0;
	laserInfo0->hitstunFrames = 0;
	laserInfo0->knockback = 0;

	laserInfo1 = new HitboxInfo;
	laserInfo1->damage = 10;
	laserInfo1->drainX = 0;
	laserInfo1->drainY = 0;
	laserInfo1->hitlagFrames = 0;
	laserInfo1->hitstunFrames = 0;
	laserInfo1->knockback = 0;

	laserInfo2 = new HitboxInfo;
	laserInfo2->damage = 20;
	laserInfo2->drainX = 0;
	laserInfo2->drainY = 0;
	laserInfo2->hitlagFrames = 0;
	laserInfo2->hitstunFrames = 0;
	laserInfo2->knockback = 0;

	laserInfo3 = new HitboxInfo;
	laserInfo3->damage = 40;
	laserInfo3->drainX = 0;
	laserInfo3->drainY = 0;
	laserInfo3->hitlagFrames = 0;
	laserInfo3->hitstunFrames = 0;
	laserInfo3->knockback = 0;

	bezLength = 60 * NUM_STEPS;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);
	cutObject->SetScale(scale);

	ResetEnemy();
}

Spider::~Spider()
{
	delete laserInfo0;
	delete laserInfo1;
	delete laserInfo2;
	delete laserInfo3;
}

void Spider::ResetEnemy()
{
	rayCastInfo.Reset();
	framesLaseringPlayer = 0;
	laserCounter = 0;
	laserLevel = 0;

	surfaceMover->Set(startPosInfo);
	surfaceMover->SetSpeed(0);
	surfaceMover->ClearAirForces();

	bezFrame = 0;
	attackFrame = -1;
	
	action = MOVE;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Spider::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 1.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 3.0;
		maxHealth += 5;
		break;
	}
}

void Spider::SetClosestLeft()
{
	double movementPossible = 1000;
	double testq = surfaceMover->edgeQuantity;
	V2d testPos;
	Edge *testEdge = surfaceMover->ground;

	V2d playerPos = sess->GetPlayerPos(0);

	while( movementPossible > 0 )
	{
		double len = length( testEdge->v1 - testEdge->v0 );

		if( movementPossible > testq )
		{
			testq = len;
			testEdge = testEdge->edge0;
			movementPossible = movementPossible - len;

			CheckClosest( testEdge, playerPos, false, testq );
		}
		else
		{
			testq = testq - movementPossible;
			movementPossible = 0;
		}
	}
}

void Spider::SetClosestRight()
{
	double movementPossible = 1000;
	double testq = surfaceMover->edgeQuantity;
	V2d testPos;
	Edge *testEdge = surfaceMover->ground;

	V2d playerPos = sess->GetPlayerPos(0);

	while( movementPossible > 0 )
	{
		double len = length( testEdge->v1 - testEdge->v0 );

		if( testq + movementPossible > len )
		{
			testq = 0;
			testEdge = testEdge->edge1;
			movementPossible = (testq + movementPossible) - len;

			CheckClosest( testEdge, playerPos, true, testq );
		}
		else
		{
			testq = movementPossible;
			movementPossible = 0;
		}



	}
}

void Spider::CheckClosest( Edge * e, V2d &playerPos,
	bool right, double cutoffQuant )
{
	double len = length( e->v1 - e->v0 );
	V2d along = normalize( e->v1 - e->v0 );

	double d = dot( playerPos - e->v0, along );
	double c = cross( playerPos - e->v0, along );

	double testQ;

	V2d possiblePos;
	if( d < 0 )
	{
		possiblePos = e->v0;

		if( !right && d < cutoffQuant )
		{
			d = cutoffQuant;
		}
		else
		{
			d = 0;
		}
		
	}
	else if( d > len )
	{
		possiblePos = e->v1;
		
		if( right && d > cutoffQuant )
		{
			d = cutoffQuant;
		}
		else
		{
			d = len;
		}
	}
	else
	{
		possiblePos = e->GetPosition( d );
	}

	if( length( playerPos - possiblePos ) 
		< length( playerPos - closestPos) )
	{
		closestPos = possiblePos;
		closestEdge = e;
		closestQuant = d;
		closestClockwiseFromCurrent = right;
	}
}

void Spider::UpdateHitboxes()
{
	Edge *ground = surfaceMover->ground;
	if( ground != NULL )
	{
		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if(surfaceMover->groundSpeed > 0 )
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
			hitboxInfo->knockback = 15;
		}
	}

	BasicUpdateHitboxes();
}

void Spider::ActionEnded()
{
	switch( action )
	{
	case MOVE:
		frame = 0;
		break;
	case JUMP:
		frame = 0;
		break;
	case LAND:
		//action = RUN;
		//frame = 0;
		break;
	case ATTACK:
		//action = RUN;
		//frame = 0;
		break;
	}
}

void Spider::ProcessState()
{
	//testLaunch->UpdatePrePhysics();
	Actor *player = sess->GetPlayer( 0 );

	if( dead )
		return;

	ActionEnded();


	closestEdge = surfaceMover->ground;
	closestQuant = surfaceMover->edgeQuantity;
	closestPos = surfaceMover->physBody.globalPosition;

	V2d playerPos = sess->GetPlayerPos(0);

	SetClosestLeft();
	SetClosestRight();
	CheckClosest(surfaceMover->ground, playerPos, true, surfaceMover->edgeQuantity );

	double len = length(playerPos - GetPosition() );
	bool outsideRange = len >= 500 && len < 1500;//1200; //bounds
	if( outsideRange && length( GetPosition() - closestPos ) > 20
		&& !canSeePlayer )
	{
		if(closestClockwiseFromCurrent)
		{
			surfaceMover->SetSpeed( 4 );
		}
		else
		{
			surfaceMover->SetSpeed( -4 );
		}
	}
	else
	{
		surfaceMover->SetSpeed( 0 );
	}

	switch( action )
	{
	case MOVE:
		//closestPos.
		break;
	case JUMP:
		break;
	case ATTACK:
		break;
	case LAND:
		break;
	}

	switch( action )
	{
	case MOVE:
		
		break;
	case JUMP:
		break;
	case ATTACK:
		{
			//mover->SetSpeed( 0 );
		}
		break;
	case LAND:
		{
			//mover->SetSpeed( 0 );
		}
		break;
	}

	if( attackFrame == 2 * attackMult )
	{
		attackFrame = -1;
	}

	frame = 0;
}


void Spider::UpdateEnemyPhysics()
{
	//if (health > 0) //!dead
	{
		surfaceMover->Move(slowMultiple, numPhysSteps);
	}
}

void Spider::FrameIncrement()
{

}

void Spider::UpdatePostPhysics()
{
	
	if( laserCounter == 0 )
	{
		HitboxInfo *currLaserInfo = NULL;
		switch( laserLevel )
		{
		case 0:
			break;
		case 1:
			currLaserInfo = laserInfo1;
			break;
		case 2:
			currLaserInfo = laserInfo2;
			break;
		case 3:
			currLaserInfo = laserInfo3;
			break;
		};

		Actor *player = sess->GetPlayer(0);
		if (player->IsInvincible())
		{

		}
		else
		{
			player->ApplyHit(currLaserInfo,
				NULL, Actor::HitResult::HIT, GetPosition());
		}

		++laserCounter;
	}
	else
	{
		if( laserCounter == 20 )
		{
			laserCounter = 0;
		}
		else
		{
			++laserCounter;
		}
	}

	V2d playerPos = sess->GetPlayerPos(0);
	if( length( playerPos - GetPosition() ) < 1200 )
	{
		rayCastInfo.rayStart = GetPosition();
		V2d laserDir( cos( laserAngle ), sin( laserAngle ) );

		rayCastInfo.rayEnd = playerPos;
		rayCastInfo.rcEdge = NULL;
		RayCast( this, sess->terrainTree->startNode, rayCastInfo.rayStart, rayCastInfo.rayEnd );

		if(rayCastInfo.rcEdge != NULL )
		{
			V2d rcPoint = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant );
			if( length( rcPoint - GetPosition() ) < length(playerPos - GetPosition() ) )
			{
				canSeePlayer = false;
			}
			else
			{
				canSeePlayer = true;
			}
		}
		else
		{
			canSeePlayer = true;
		}
	}
	else
	{
		canSeePlayer = false;
	}

	if( canSeePlayer )
	{
		framesLaseringPlayer += 1;
	}
	else
	{
		framesLaseringPlayer = 0;
		//laserCounter = 0;
	}

	if( framesLaseringPlayer > 240 )
	{
		if( laserLevel != 3 )
		{
			laserLevel = 3;
			laserCounter = 0;
		}
		
	}
	else if( framesLaseringPlayer > 180 )
	{
		if( laserLevel != 2 )
		{
			laserLevel = 2;
			laserCounter = 0;
		}
	}
	else if( framesLaseringPlayer > 120 )
	{
		if( laserLevel != 1 )
		{
			laserLevel = 1;
			laserCounter = 0;
		}
	}
	else
	{
		laserLevel = 0;
		laserCounter = 0;
	}

	Enemy::UpdatePostPhysics();
}

void Spider::EnemyDraw(sf::RenderTarget *target )
{
	DrawSprite(target, sprite);

	CircleShape cs;
	cs.setRadius(20);
	cs.setFillColor(Color::Magenta);
	cs.setOrigin(cs.getLocalBounds().width / 2,
		cs.getLocalBounds().height / 2);
	cs.setPosition(Vector2f( closestPos));
	target->draw(cs);

	Color laserColor;
	switch (laserLevel)
	{
	case 0:
		laserColor = Color::White;
		break;
	case 1:
		laserColor = Color::Green;
		break;
	case 2:
		laserColor = Color::Red;
		break;
	case 3:
		laserColor = Color::Magenta;
		break;
	}

	if (canSeePlayer)
	{
		V2d rcPoint;
		if (rayCastInfo.rcEdge != NULL)
		{
			rcPoint = rayCastInfo.rcEdge->GetPosition(rayCastInfo.rcQuant);
		}
		else
		{
			rcPoint = rayCastInfo.rayEnd;
		}
		sf::Vertex line[] = {
			Vertex(GetPositionF(), laserColor),
			Vertex(Vector2f(rcPoint.x, rcPoint.y)
			, laserColor)
		};
		target->draw(line, 2, sf::Lines);
	}
	else
	{
		/*V2d rcPoint = rcEdge->GetPoint( rcQuantity );
		sf::Vertex line[]= {
		Vertex( Vector2f( position.x, position.y )
		, Color::Red )
		, Vertex( Vector2f( rcPoint.x, rcPoint.y ), Color::Red )
		};
		target->draw( line, 2, sf::Lines );*/
	}
}

 
void Spider::UpdateSprite()
{
	IntRect ir = ts->GetSubRect(0);

	ts->SetSubRect(sprite, 0, !facingRight, false);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

bool Spider::StartRoll()
{
	return false;
}

void Spider::FinishedRoll()
{
}

void Spider::HitTerrain( double &q )
{
	
}

void Spider::HitOther()
{
	V2d v;
	if( facingRight && surfaceMover->groundSpeed > 0 )
	{
		v = V2d( 10, -10 );
		surfaceMover->Jump( v );
	}
	else if( !facingRight && surfaceMover->groundSpeed < 0 )
	{
		v = V2d( -10, -10 );
		surfaceMover->Jump( v );
	}
	//cout << "hit other!" << endl;
	//mover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void Spider::ReachCliff()
{
	if( facingRight && surfaceMover->groundSpeed < 0
		|| !facingRight && surfaceMover->groundSpeed > 0 )
	{
		surfaceMover->SetSpeed( 0 );
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;
	V2d v;
	if( facingRight )
	{
		v = V2d( 10, -10 );
	}
	else
	{
		v = V2d( -10, -10 );
	}

	surfaceMover->Jump( v );
	//mover->groundSpeed = -mover->groundSpeed;
	//facingRight = !facingRight;
}

void Spider::HitOtherAerial( Edge *e )
{
	//cout << "hit edge" << endl;
}

void Spider::Land()
{
	frame = 0;
	//cout << "land" << endl;
}

void Spider::HandleRayCollision( Edge *edge, double equant, double rayPortion )
{
	if( edge->edgeType == Edge::OPEN_GATE )
	{
		return;
	}

	RayCastHandler::HandleRayCollision(edge, equant, rayPortion);
}