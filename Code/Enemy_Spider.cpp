#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Spider.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

Spider::Spider( GameSession *owner, bool p_hasMonitor, Edge *g, double q, int speed )
	:Enemy( owner, EnemyType::EN_SPIDER, p_hasMonitor, 1 ), facingRight( true )
	//moveBezTest( .22,.85,.3,.91 )
{
	rcEdge = NULL;
	gravity = V2d( 0, .6 );
	maxGroundSpeed = speed;
	action = MOVE;
	maxFallSpeed = 25;

	//ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	//ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );

	attackFrame = -1;
	attackMult = 10;

	double height = 128;
	double width = 128;

	startGround = g;
	startQuant = q;
	frame = 0;

	mover = new SurfaceMover( owner, g, q, 32 );
	//mover->gravity = V2d( 0, .5 );
	mover->SetSpeed( 0 );
	//mover->groundSpeed = s;
	/*if( !facingRight )
	{
		mover->groundSpeed = -mover->groundSpeed;
	}*/

	ts = owner->GetTileset( "Enemies/crawler_160x160.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = g->GetPoint( q );
	sprite.setPosition( mover->physBody.globalPosition.x,
		mover->physBody.globalPosition.y );
	position = mover->physBody.globalPosition;
	//roll = false;
	//position = gPoint + ground->Normal() * height / 2.0;
	

	receivedHit = NULL;

	double size = max( width, height );
	spawnRect = sf::Rect<double>( gPoint.x - size, gPoint.y - size, size * 2, size * 2 );

	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 32;
	hurtBox.rh = 32;
	hurtBody = new CollisionBody(1);
	hurtBody->AddCollisionBox(0, hurtBox);

	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody = new CollisionBody(1);
	hitBody->AddCollisionBox(0, hitBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 0;
	hitboxInfo->knockback = 0;

	hitBody->hitboxInfo = hitboxInfo;

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

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;

	bezLength = 60 * NUM_STEPS;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);

	ResetEnemy();
}

void Spider::ResetEnemy()
{
	rcEdge = NULL;
	framesLaseringPlayer = 0;
	laserCounter = 0;
	laserLevel = 0;

	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed( 0 );

	position = mover->physBody.globalPosition;

	bezFrame = 0;
	attackFrame = -1;
	V2d gPoint = mover->ground->GetPoint( mover->edgeQuantity );
	frame = 0;

	V2d gn = mover->ground->Normal();
	dead = false;

	double angle = 0;
	angle = atan2( gn.x, -gn.y );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( gPoint.x, gPoint.y );

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	UpdateHitboxes();
}

void Spider::SetClosestLeft()
{
	double movementPossible = 1000;
	double testq = mover->edgeQuantity;
	V2d testPos;
	Edge *testEdge = mover->ground;

	V2d playerPos = owner->GetPlayer( 0 )->position;

	while( movementPossible > 0 )
	{
		double len = length( testEdge->v1 - testEdge->v0 );
		/*V2d along = normalize( testEdge->v1 - testEdge->v0 );
		
		double d = dot( playerPos - testEdge->v0, along );
		double c = cross( playerPos - testEdge->v0, along );*/

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
	double testq = mover->edgeQuantity;
	V2d testPos;
	Edge *testEdge = mover->ground;

	V2d playerPos = owner->GetPlayer( 0 )->position;

	while( movementPossible > 0 )
	{
		double len = length( testEdge->v1 - testEdge->v0 );
		/*V2d along = normalize( testEdge->v1 - testEdge->v0 );
		
		double d = dot( playerPos - testEdge->v0, along );
		double c = cross( playerPos - testEdge->v0, along );*/

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
		possiblePos = e->GetPoint( d );
	}

	if( length( playerPos - possiblePos ) 
		< length( playerPos - closestPos.position ) )
	{
		closestPos.position = possiblePos;
		closestPos.e = e;
		closestPos.q = d;
		closestPos.clockwiseFromCurrent = right;
	}
}

void Spider::UpdateHitboxes()
{
	Edge *ground = mover->ground;
	if( ground != NULL )
	{
		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if( mover->groundSpeed > 0 )
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

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	hitBody->GetCollisionBoxes(0)->front().globalPosition = mover->physBody.globalPosition;
	hurtBody->GetCollisionBoxes(0)->front().globalPosition = mover->physBody.globalPosition;
	//physBody.globalPosition = position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
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
	Actor *player = owner->GetPlayer( 0 );

	if( dead )
		return;

	ActionEnded();


	closestPos.e = mover->ground;
	closestPos.q = mover->edgeQuantity;
	closestPos.position = mover->physBody.globalPosition;

	SetClosestLeft();
	SetClosestRight();
	CheckClosest( mover->ground, player->position, true, mover->edgeQuantity );

	double len = length( player->position - position );
	bool outsideRange = len >= 500 && len < 1500;//1200; //bounds
	if( outsideRange && length( position - closestPos.position ) > 20
		&& !canSeePlayer )
	{
		if( closestPos.clockwiseFromCurrent )
		{
			mover->SetSpeed( 4 );
		}
		else
		{
			mover->SetSpeed( -4 );
		}
	}
	else
	{
		mover->SetSpeed( 0 );
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
		mover->Move(slowMultiple, numPhysSteps);
		position = mover->physBody.globalPosition;
	}
}

void Spider::FrameIncrement()
{

}

void Spider::UpdatePostPhysics()
{
	if( laserCounter == 0 )
	{
		switch( laserLevel )
		{
		case 0:
			break;
		case 1:
			owner->GetPlayer( 0 )->ApplyHit( laserInfo1 );
			//owner->GetPlayer( 0 )->app
			break;
		case 2:
			owner->GetPlayer( 0 )->ApplyHit( laserInfo2 );
			break;
		case 3:
			owner->GetPlayer( 0 )->ApplyHit( laserInfo3 );
			break;
		};
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

	if( length( owner->GetPlayer( 0 )->position - mover->physBody.globalPosition ) < 1200 )
	{
		rayStart = mover->physBody.globalPosition;
		V2d laserDir( cos( laserAngle ), sin( laserAngle ) );

		//rayEnd = rayStart + laserDir * 1000.0;//owner->GetPlayer( 0 )->position;
		rayEnd = owner->GetPlayer( 0 )->position;
		rcEdge = NULL;
		RayCast( this, owner->terrainTree->startNode, rayStart, rayEnd );

		if( rcEdge != NULL )
		{
			V2d rcPoint = rcEdge->GetPoint( rcQuantity );
			if( length( rcPoint - position ) < length( owner->GetPlayer( 0 )->position - position ) )
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
	DrawSpriteIfExists(target, sprite);

	CircleShape cs;
	cs.setRadius(20);
	cs.setFillColor(Color::Magenta);
	cs.setOrigin(cs.getLocalBounds().width / 2,
		cs.getLocalBounds().height / 2);
	cs.setPosition(closestPos.position.x,
		closestPos.position.y);
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
		if (rcEdge != NULL)
		{
			rcPoint = rcEdge->GetPoint(rcQuantity);
		}
		else
		{
			rcPoint = rayEnd;
		}
		sf::Vertex line[] = {
			Vertex(Vector2f(position.x, position.y), laserColor),
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

	//sprite.setTextureRect(ir);
	/*if( attackFrame >= 0 )
	{
		ir = ts->GetSubRect( 28 + attackFrame / attackMult );
		if( !facingRight )
		{
			ir = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
		}
		sprite.setTextureRect( ir );
	}
	else
	{
		
	}*/

	if (!facingRight)
	{
		sprite.setTextureRect(sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height));
	}
	else
	{
		sprite.setTextureRect(ir);
	}

	double angle;
	V2d gn = mover->ground->Normal();

	if (!mover->roll)
	{
		angle = atan2(gn.x, -gn.y);

		V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
		sprite.setRotation(angle / PI * 180);
		sprite.setPosition(pp.x, pp.y);
	}
	else
	{
		if (facingRight)
		{
			V2d vec = normalize(position - mover->ground->v1);
			angle = atan2(vec.y, vec.x);
			angle += PI / 2.0;

			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation(angle / PI * 180);
			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);//ground->GetPoint( edgeQuantity );
			sprite.setPosition(pp.x, pp.y);
		}
		else
		{
			V2d vec = normalize(position - mover->ground->v0);
			angle = atan2(vec.y, vec.x);
			angle += PI / 2.0;

			sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation(angle / PI * 180);
			V2d pp = mover->ground->GetPoint(mover->edgeQuantity);
			sprite.setPosition(pp.x, pp.y);
		}
	}
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
	if( facingRight && mover->groundSpeed > 0 )
	{
		v = V2d( 10, -10 );
		mover->Jump( v );
	}
	else if( !facingRight && mover->groundSpeed < 0 )
	{
		v = V2d( -10, -10 );
		mover->Jump( v );
	}
	//cout << "hit other!" << endl;
	//mover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void Spider::ReachCliff()
{
	if( facingRight && mover->groundSpeed < 0 
		|| !facingRight && mover->groundSpeed > 0 )
	{
		mover->SetSpeed( 0 );
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

	mover->Jump( v );
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

	if( rcEdge == NULL || length( edge->GetPoint( equant ) - rayStart ) < 
		length( rcEdge->GetPoint( rcQuantity ) - rayStart ) )
	{
		rcEdge = edge;
		rcQuantity = equant;
	}
}