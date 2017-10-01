#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

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
	:Enemy( owner, EnemyType::SPIDER, p_hasMonitor, 4 ), facingRight( true )
	//moveBezTest( .22,.85,.3,.91 )
{

	framesLaseringPlayer = 0;
	rcEdge = NULL;
	gravity = V2d( 0, .6 );
	maxGroundSpeed = speed;
	action = MOVE;
	initHealth = 60;
	health = initHealth;
	dead = false;
	deathFrame = 0;

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

	ts = owner->GetTileset( "crawler_128x128.png", width, height );
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

	hurtBody.type = CollisionBox::Hurt;
	hurtBody.isCircle = true;
	hurtBody.globalAngle = 0;
	hurtBody.offset.x = 0;
	hurtBody.offset.y = 0;
	hurtBody.rw = 32;
	hurtBody.rh = 32;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 32;
	hitBody.rh = 32;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 0;
	hitboxInfo->knockback = 0;

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

	laserCounter = 0;

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;


	/*testLaunch = new Launcher( this, owner, 10, 1,
		mover->physBody.globalPosition, g->Normal(), 0 );*/
	/*physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 32;
	physBody.rh = 32;
	physBody.type = CollisionBox::BoxType::Physics;*/

	

	deathPartingSpeed = .4;

	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

	

	bezFrame = 0;
	bezLength = 60 * NUM_STEPS;

	mover->SetSpeed( 0 );
	//mover->Move( slowMultiple );

	//ground = mover->ground;
	//edgeQuantity = mover->edgeQuantity;
	//position = mover->physBody.globalPosition;
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
	//mover->UpdateGroundPos();

	//testLaunch->Reset();
	//testLaunch->position = mover->physBody.globalPosition;
	//testLaunch->facingDir = startGround->Normal();

	bezFrame = 0;
	health = initHealth;
	attackFrame = -1;
	//lastReverser = false;
	//roll = false;
	//ground = startGround;
	//edgeQuantity = startQuant;
	V2d gPoint = mover->ground->GetPoint( mover->edgeQuantity );
	//sprite.setPosition( mover->physBody.globalPosition.x,
	//	mover->physBody.globalPosition.y );
	frame = 0;

	V2d gn = mover->ground->Normal();
	//mover->physBody.globalPosition = gPoint + mover->ground->Normal() * 64.0 / 2.0;

	/*V2d gn = ground->Normal();
	if( gn.x > 0 )
		offset.x = physBody.rw;
	else if( gn.x < 0 )
		offset.x = -physBody.rw;
	if( gn.y > 0 )
		offset.y = physBody.rh;
	else if( gn.y < 0 )
		offset.y = -physBody.rh;*/

	//position = gPoint + offset;

	deathFrame = 0;
	dead = false;

	//----update the sprite
	double angle = 0;
	////position = gPoint + gn * 32.0;
	angle = atan2( gn.x, -gn.y );
	//	
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setRotation( angle );
	//sprite.setTextureRect( ts->GetSubRect( frame / crawlAnimationFactor ) );
	//sprite.setPosition( 
	//V2d pp = ground->GetPoint( edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( gPoint.x, gPoint.y );
	//----

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

int Spider::NumTotalBullets()
{
	return 0;
}

void Spider::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	//might need for other queries but def not for physics
}

void Spider::UpdateHitboxes()
{
	Edge *ground = mover->ground;
	if( ground != NULL )
	{
		//V2d gn = ground->Normal();
		//double angle = 0;
		
		
		//angle = atan2( gn.x, -gn.y );
		
		//hitBody.globalAngle = angle;
		//hurtBody.globalAngle = angle;

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
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	hitBody.globalPosition = mover->physBody.globalPosition;
	hurtBody.globalPosition = mover->physBody.globalPosition;
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

void Spider::UpdatePrePhysics()
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

	//V2d playerDir = normalize( player->position - mover->physBody.globalPosition );
	//double playerAngle = atan2( playerDir.y, playerDir.x );

	//if( playerAngle < 0 )
	//{
	//	playerAngle += PI * 2;
	//}
	//
	//double rate = PI / 360.0;
	//double window = PI / 60.0;
	//if( playerAngle - laserAngle > window )
	//{
	//	if( playerAngle - laserAngle > PI )
	//	{
	//		laserAngle -= rate;
	//	}
	//	else
	//	{
	//		laserAngle += rate;
	//	}
	//}
	//else if( playerAngle - laserAngle < -window) //some diff range
	//{
	//	if( laserAngle - playerAngle > PI )
	//	{
	//		laserAngle += rate;
	//	}
	//	else
	//	{
	//		laserAngle -= rate;
	//	}
	//}

	//if( laserAngle < 0 )
	//{
	//	laserAngle += PI * 2;
	//}
	//else if( laserAngle > PI * 2 )
	//{
	//	laserAngle -= PI * 2;
	//}

	
	//CheckClosest( mover->ground, player->position, false, mover->edgeQuantity );

	//cout << "closest pos is: " << closestPos.position.x << ", "
	//	<< closestPos.position.y << endl;

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


	bool roll = mover->roll;

	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			dead = true;

			
		}

		receivedHit = NULL;
	}


	
	if( attackFrame == 2 * attackMult )
	{
		attackFrame = -1;
	}
	//if( attacking )
	//{
	//}
	//else
	//{

	frame = 0;
	/*if( !roll && frame == 16 * crawlAnimationFactor )
	{
		frame = 0;
	}

	if ( roll && frame == 10 * rollAnimationFactor )
	{
		frame = rollAnimationFactor * 2; 
	}*/

		//cout << "groundspeed: " << mover->groundSpeed << endl;
	//}

	/*if( bezFrame == 0 )
	{
		testLaunch->position = position;
		if( mover->ground != NULL )
		{
			testLaunch->facingDir = mover->ground->Normal();
		}
		else
		{
			testLaunch->facingDir = V2d( 0, -1 );
		}
		
		testLaunch->Fire();
	}*/
}

void Spider::UpdatePhysics()
{
	//testLaunch->UpdatePhysics();
	specterProtected = false;

	if( dead )
	{
		return;
	}



	double f = moveBezTest.GetValue( bezFrame / (double)bezLength );
	//mover->groundSpeed = groundSpeed;// * f;
	if( !facingRight )
	{
	//	mover->groundSpeed = groundSpeed;// * f;
	}
	bezFrame++;

	if( bezFrame == bezLength )
	{
		bezFrame = 0;
		

	}

	if( mover->ground != NULL )
	{
	}
	else
	{
		mover->velocity += gravity / (NUM_STEPS * slowMultiple);

		if( mover->velocity.y >= maxFallSpeed )
		{
			mover->velocity.y = maxFallSpeed;
		}
	}

	
	//mover->groundSpeed = 5;
	mover->Move( slowMultiple );

	position = mover->physBody.globalPosition;
	
	PhysicsResponse();
}

bool Spider::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;

	double rw = mover->physBody.rw;
	double rh = mover->physBody.rh;

	Rect<double> oldR( position.x - rw, 
		position.y - rh, 2 * rw, 2 * rh );
	position += vel;
	
	Rect<double> newR( position.x - rw, 
		position.y - rh, 2 * rw, 2 * rh );
	//minContact.collisionPriority = 1000000;
	
	double oldRight = oldR.left + oldR.width;
	double right = newR.left + newR.width;

	double oldBottom = oldR.top + oldR.height;
	double bottom = newR.top + newR.height;

	double maxRight = max( right, oldRight );
	double maxBottom = max( oldBottom, bottom );
	double minLeft = min( oldR.left, newR.left );
	double minTop = min( oldR.top, newR.top );
	//Rect<double> r( minLeft - 5 , minTop - 5, maxRight - minLeft + 5, maxBottom - minTop + 5 );
	Rect<double> r( minLeft , minTop, maxRight - minLeft, maxBottom - minTop );

	
	minContact.collisionPriority = 1000000;

	

	tempVel = vel;

	col = false;
	minContact.edge = NULL;

	queryMode = "resolve";
	owner->terrainTree->Query( this, r );
	//Query( this, owner->testTree, r );

	return col;
}

void Spider::PhysicsResponse()
{
	if( !dead  )
	{
		bool roll = mover->roll;
		double angle = 0;
		Edge *ground = mover->ground;
		double edgeQuantity = mover->edgeQuantity;

		if( ground != NULL )
		{
		//cout << "response" << endl;
			double spaceNeeded = 0;
			V2d gn = ground->Normal();
			V2d gPoint = ground->GetPoint( edgeQuantity );
	

		
	
		if( !roll )
		{
			//position = gPoint + gn * 32.0;
			angle = atan2( gn.x, -gn.y );
		
//			sprite.setTexture( *ts_walk->texture );
			IntRect r = ts->GetSubRect( frame / crawlAnimationFactor );
			if( !facingRight )
			{
				sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
			}
			else
			{
				sprite.setTextureRect( r );
			}
			
			//V2d pp = ground->GetPoint( mover->edgeQuantity );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( gPoint.x, gPoint.y );
		}
		else
		{
			
			if( facingRight )
			{
				V2d vec = normalize( position - ground->v1 );
				angle = atan2( vec.y, vec.x );
				angle += PI / 2.0;
	

				//sprite.setTexture( *ts->texture );
				IntRect r = ts->GetSubRect( frame / rollAnimationFactor + 17 );
				if( facingRight )
				{
					sprite.setTextureRect( r );
				}
				else
				{
					sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
				}
			
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				sprite.setPosition( gPoint.x, gPoint.y );
			}
			else
			{
				//angle = 
				/*V2d e0n = ground->edge0->Normal();
				double rollStart = atan2( gn.y, gn.x );
				double rollEnd = atan2( e0n.y, e0n.x );
				double adjRollStart = rollStart;
				double adjRollEnd = rollEnd;

				if( rollStart < 0 )
					adjRollStart += 2 * PI;
				if( rollEnd < 0 )
					adjRollEnd += 2 * PI;
		
				if( adjRollEnd > adjRollStart )
				{
					angle  = adjRollStart * ( 1.0 - rollFactor ) + adjRollEnd  * rollFactor ;
				}
				else
				{
			
					angle = rollStart * ( 1.0 - rollFactor ) + rollEnd  * rollFactor;

					if( rollStart < 0 )
						rollStart += 2 * PI;
					if( rollEnd < 0 )
						rollEnd += 2 * PI;
				}

				if( angle < 0 )
					angle += PI * 2;*/

			

			//	V2d angleVec = V2d( cos( angle ), sin( angle ) );
			//	angleVec = normalize( angleVec );

			//	position = gPoint + angleVec * 16.0;
				V2d vec = normalize( position - ground->v0 );
				angle = atan2( vec.y, vec.x );
				angle += PI / 2.0;
	

				//sprite.setTexture( *ts->texture );
				IntRect r = ts->GetSubRect( frame / rollAnimationFactor + 17 );
				if( facingRight )
				{
					sprite.setTextureRect( r );
				}
				else
				{
					sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
				}
			
				sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
				sprite.setRotation( angle / PI * 180 );
				sprite.setPosition( gPoint.x, gPoint.y );
			}	
		}
		}
		else
		{
			V2d p = mover->physBody.globalPosition;

			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height * 3.0/4.0);
			sprite.setPosition( p.x, p.y );
			sprite.setRotation( 0 );
		}

		//sprite.setPosition( position.x, position.y );

		UpdateHitboxes();

		if( PlayerSlowingMe() )
		{
			if( slowMultiple == 1 )
			{
				slowCounter = 1;
				slowMultiple = 5;
			//	cout << "yes slow" << endl;
			}
		}
		else
		{
			slowCounter = 1;
			slowMultiple = 1;
		//	cout << "no slow" << endl;
		}

		if( receivedHit == NULL )
		{
			pair<bool, bool> result = PlayerHitMe();
			if( result.first )
			{
				//cout << "hit here!" << endl;
				//triggers multiple times per frame? bad?
				owner->GetPlayer( 0 )->ConfirmHit( 4, 5, .8, 6 );

				if( owner->GetPlayer( 0 )->ground == NULL && owner->GetPlayer( 0 )->velocity.y > 0 )
				{
					owner->GetPlayer( 0 )->velocity.y = 4;//.5;
				}

															//cout << "frame: " << owner->GetPlayer( 0 )->frame << endl;

			//owner->GetPlayer( 0 )->frame--;
			//owner->ActivateEffect( ts_testBlood, position, true, 0, 6, 3, facingRight );
		//	cout << "patroller received damage of: " << receivedHit->damage << endl;
			
			/*if( !result.second )
			{
				owner->Pause( 6 );
			}*/
			
			//dead = true;
			//receivedHit = NULL;
			}
		}

		if( IHitPlayer() )
		{
		//	cout << "patroller just hit player for " << hitboxInfo->damage << " damage!" << endl;
		}

		//gotta get the correct angle upon death
		Transform t;
		t.rotate( angle / PI * 180 );
		Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
		deathVector = V2d( newPoint.x, newPoint.y );

		queryMode = "reverse";

		//physbody is a circle
		//Rect<double> r( position.x - physBody.rw, position.y - physBody.rw, physBody.rw * 2, physBody.rw * 2 );
		//owner->crawlerReverserTree->Query( this, r );
	}
}

void Spider::UpdatePostPhysics()
{
	if( receivedHit != NULL )
		owner->Pause( 5 );

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
	}
	else
	{
		++laserCounter;
		if( laserCounter == 20 )
		{
			laserCounter = 0;
		}
	}

	if( deathFrame == 30 )
	{
		owner->RemoveEnemy( this );
		return;
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

	UpdateSprite();
	//testLaunch->UpdateSprites();

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
		
		if( dead )
		{
			deathFrame++;
		}
		else
		{
			if( attackFrame >= 0 )
				++attackFrame;
		}
	}
	else
	{
		slowCounter++;
	}

	//cout << "position: " << position.x << ", " << position.y << endl;
	//need to calculate frames in here!!!!

	//sprite.setPosition( position );
	//UpdateHitboxes();
}

bool Spider::PlayerSlowingMe()
{
	Actor *player = owner->GetPlayer( 0 );
	for( int i = 0; i < player->maxBubbles; ++i )
	{
		if( player->bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player->bubblePos[i] ) <= player->bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void Spider::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		if( hasMonitor && !suppressMonitor )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 55 );
			cs.setFillColor( Color::Black );
			
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		target->draw( sprite );

		if( !dead )
		{
			CircleShape cs;
			cs.setRadius( 20 );
			cs.setFillColor( Color::Magenta );
			cs.setOrigin( cs.getLocalBounds().width / 2, 
				cs.getLocalBounds().height / 2 );
			cs.setPosition( closestPos.position.x, 
				closestPos.position.y );
			target->draw( cs );

			Color laserColor;
			switch( laserLevel )
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

			if( canSeePlayer )
			{
				V2d rcPoint;
				if( rcEdge != NULL )
				{
					rcPoint = rcEdge->GetPoint( rcQuantity );
				}
				else
				{
					rcPoint = rayEnd;
				}
				sf::Vertex line[]= { 
					Vertex( Vector2f( position.x, position.y ), laserColor ),
					Vertex( Vector2f( rcPoint.x, rcPoint.y )
					, laserColor )
				};
				target->draw( line, 2, sf::Lines );
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
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );
		}
		
		target->draw( topDeathSprite );
	}
}

void Spider::DrawMinimap( sf::RenderTarget *target )
{
	if( !dead )
	{
		if( hasMonitor && !suppressMonitor )
		{
			CircleShape cs;
			cs.setRadius( 50 );
			cs.setFillColor( Color::White );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
		else
		{
			CircleShape cs;
			cs.setRadius( 40 );
			cs.setFillColor( Color::Red );
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
	}

	/*CircleShape cs;
	cs.setRadius( 50 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );*/

	/*if( hasMonitor && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/
}

bool Spider::IHitPlayer( int index )
{
	Actor *player = owner->GetPlayer( 0 );
	
	if( player->invincibleFrames == 0 && hitBody.Intersects( player->hurtBody ) )
	{
		if( player->position.x < position.x )
		{
			hitboxInfo->kbDir.x = -abs( hitboxInfo->kbDir.x );
			//cout << "left" << endl;
		}
		else if( player->position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir.x = abs( hitboxInfo->kbDir.x );
		}
		else
		{
			//dont change it
		}
		attackFrame = 0;
		player->ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

 pair<bool, bool> Spider::PlayerHitMe( int index )
{
	Actor *player = owner->GetPlayer( 0 );

	if( player->currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player->currHitboxes->begin(); it != player->currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			sf::Rect<double> qRect( position.x - hurtBody.rw,
			position.y - hurtBody.rw, hurtBody.rw * 2, 
			hurtBody.rw * 2 );
			owner->specterTree->Query( this, qRect );

			if( !specterProtected )
			{
				receivedHit = player->currHitboxInfo;
				return pair<bool, bool>(true,false);
			}
			else
			{
				return pair<bool, bool>(false,false);
			}
			
		}
		
	}

	for( int i = 0; i < player->recordedGhosts; ++i )
	{
		if( player->ghostFrame < player->ghosts[i]->totalRecorded )
		{
			if( player->ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player->ghosts[i]->currHitboxes->begin(); it != player->ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player->currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player->ghosts[i]->curhi
		}
	}
	return pair<bool, bool>(false,false);
}

void Spider::UpdateSprite()
{
	if( dead )
	{
		//cout << "deathVector: " << deathVector.x << ", " << deathVector.y << endl;
		botDeathSprite.setTexture( *ts->texture );
		botDeathSprite.setTextureRect( ts->GetSubRect( 31 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts->texture );
		topDeathSprite.setTextureRect( ts->GetSubRect( 30 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
	else
	{
		if( attackFrame >= 0 )
		{
			IntRect r = ts->GetSubRect( 28 + attackFrame / attackMult );
			if( !facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );
		}
	}
}

void Spider::DebugDraw( RenderTarget *target )
{
	if( !dead )
	{
		//if( ground != NULL )
		{
		/*CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		V2d g = ground->GetPoint( edgeQuantity );
		cs.setPosition( g.x, g.y );*/
		}
		//owner->window->draw( cs );
		//UpdateHitboxes();
		//physBody.DebugDraw( target );
		mover->physBody.DebugDraw( target );
	}
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void Spider::SaveEnemyState()
{
}

void Spider::LoadEnemyState()
{
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