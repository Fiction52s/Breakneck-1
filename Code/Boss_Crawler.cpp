#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Boss.h"

using namespace std;
using namespace sf;

#define V2d sf::Vector2<double>

#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

Boss_Crawler::Boss_Crawler( GameSession *owner, Edge *g, double q )
	:Enemy( owner, EnemyType::STAGBEETLE, false, 1 ), facingRight( true ),
	markerVA( sf::Quads, 5 * 4 )
	//moveBezTest( .22,.85,.3,.91 )
{
	shootIndex = 0;
	numBullets = 5;
	//5 is max bullets
	for( int i = 0; i < 5 * 4; ++i )
	{
		markerVA[i].position = Vector2f( 0, 0 );
		markerVA[i].color = Color::Red;
	}
	
	//22.59
	bulletIndex = 0;
	frameTest = 0;	
	action = SHOOT;
	travelIndex = 0;

	SetDirs();






	initHealth = 60;
	health = initHealth;
	dead = false;
	deathFrame = 0;

	//double height = 128;
	//double width = 128;

	startGround = g;
	startQuant = q;
	frame = 0;

	mover = new SurfaceMover( owner, g, q, 64 );
	mover->surfaceHandler = this;
	mover->SetSpeed( 0 );
	

	//ts = owner->GetTileset( "crawler_128x128.png", width, height );
	double width = 128;
	double height = 144;
	ts = owner->GetTileset( "bosscrawler_128x144.png", width, height );
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
	hurtBody.rw = 64;
	hurtBody.rh = 64;

	hitBody.type = CollisionBox::Hit;
	hitBody.isCircle = true;
	hitBody.globalAngle = 0;
	hitBody.offset.x = 0;
	hitBody.offset.y = 0;
	hitBody.rw = 64;
	hitBody.rh = 64;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	//crawlAnimationFactor = 5;
	//rollAnimationFactor = 5;


	/*testLaunch = new Launcher( this, owner, 10, 1,
		mover->physBody.globalPosition, g->Normal(), 0 );*/
	/*physBody.isCircle = true;
	physBody.offset.x = 0;
	physBody.offset.y = 0;
	physBody.rw = 32;
	physBody.rh = 32;
	physBody.type = CollisionBox::BoxType::Physics;*/

	launcher = new Launcher( this, BasicBullet::BOSS_CRAWLER, owner, 16, 1, position, V2d( 1, 0 ), 0, 300 );
	launcher->SetBulletSpeed( 10 );	

	deathPartingSpeed = .4;

	ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	bloodSprite.setTexture( *ts_testBlood->texture );

	totalDistanceAround = 0;
	Edge *curr = mover->ground;
	do
	{
		
		totalDistanceAround += length( curr->v1 - curr->v0 );
		cout << "total add: " << totalDistanceAround << ", " << curr << endl;
		curr = curr->edge1;
	}
	while( curr != mover->ground );

	

	//bezFrame = 0;
	//bezLength = 60 * NUM_STEPS;

	//mover->SetSpeed( 0 );
	//mover->Move( slowMultiple );

	//ground = mover->ground;
	//edgeQuantity = mover->edgeQuantity;
	//position = mover->physBody.globalPosition;
}

void Boss_Crawler::SetDirs()
{
	int possibles[16];
	for( int i = 0; i < 16; ++i )
	{
		possibles[i] = i;
	}

	//swap
	for (int i = 0; i < 16; ++i)
	{
		int j = rand() % 16;
		int temp = possibles[i];
		possibles[i] = possibles[j];
		possibles[j] = temp;
	}

	for( int i =0; i < 5; ++i )
	{
		bulletDirIndex[i] = possibles[i];
	}
}

void Boss_Crawler::ResetEnemy()
{
	
	SetDirs();

	shootIndex = 0;
	launcher->Reset();
	bulletIndex = 0;
	frameTest = 0;	
	action = SHOOT;
	travelIndex = 0;

	for( int i = 0; i < 5 * 4; ++i )
	{
		markerVA[i].position = Vector2f( 0, 0 );
		markerVA[i].color = Color::Red;
	}

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

int Boss_Crawler::NumTotalBullets()
{
	return 0;
}

void Boss_Crawler::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	//might need for other queries but def not for physics
}

void Boss_Crawler::UpdateHitboxes()
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

void Boss_Crawler::ActionEnded()
{
}

double Boss_Crawler::GetDistanceCCW( int index )
{
	Edge *e = bulletHits[index].edge;
	double q = bulletHits[index].quantity;

	
	double sum = 0;
	Edge *curr = mover->ground;
	double currQ = mover->edgeQuantity;

	sum += currQ;
	curr = curr->edge0;

	while( curr != e )
	{
		sum += length( curr->v1 - curr->v0 );	
		curr = curr->edge0;
	}

	sum += length( curr->v1 - curr->v0 ) - q;

	return sum;
}

double Boss_Crawler::GetDistanceClockwise( int index )
{
	Edge *e = bulletHits[index].edge;
	double q = bulletHits[index].quantity;

	
	double sum = 0;
	Edge *curr = mover->ground;
	double currQ = mover->edgeQuantity;

	sum += length( curr->v1 - curr->v0 ) - currQ;
	curr = curr->edge1;

	while( curr != e )
	{
		sum += length( curr->v1 - curr->v0 );	
		//cout << "sum clockwise: " << sum << ", " << curr << endl;
		curr = curr->edge1;
	}

	sum += q;


	return sum;
}

bool Boss_Crawler::GetClockwise( int index )
{
	Edge *e = bulletHits[index].edge;
	double q = bulletHits[index].quantity;

	
	double sum = 0;
	Edge *curr = mover->ground;
	double currQ = mover->edgeQuantity;

	if( curr == e )
	{
		if( q > currQ )
		{
			cout << "false same edge" << endl;
			return false;
		}
		else
		{
			cout << "true same edge" << endl;
			return true;
		}
	}
	else
	{
		double distanceClockwise = GetDistanceClockwise( index );
		double ccw = GetDistanceCCW( index );
		if( distanceClockwise >= ccw )
		{
			cout << "clockwise is greater: " << distanceClockwise << ", " << ccw << endl;
			cout << "sum: " << distanceClockwise + ccw << endl;
			//cout << "distance clock: " << distanceClockwise << ", " <<
			//	totalDistanceAround / 2 << endl;
			return true;
		}
		else
		{
			//cout << "distance clock false: " << distanceClockwise << ", blah: " << blah << ", "
			//	<< totalDistanceAround / 2 << endl;
			cout << "ccw is greater: " << distanceClockwise << ", " << ccw <<endl;
			cout << "sum: " << distanceClockwise + ccw << endl;
			return false;
		}
	}
}



void Boss_Crawler::UpdatePrePhysics()
{
	launcher->UpdatePrePhysics();
	Actor *player = owner->player;

	if( dead )
		return;

	ActionEnded();

	if( action == BOOST )
	{
		/*frameTest++;
		if( frameTest == 60 )
		{
			action = SHOOT;
			frameTest = 0;
		}*/
	}
	else if( action == SHOOT )
	{
		if( frameTest == 250 )
		{
			action = BOOST;
			travelIndex = 0;
			frameTest = 0;
			leftFirstEdge = false;
			firstEdge = mover->ground;

			if( GetClockwise( travelIndex ) )
			{
				mover->SetSpeed( 20 + travelIndex * 8 );
			}
			else
			{
				mover->SetSpeed( -20 - travelIndex * 8 );
			}
		}
	}




	if( action == BOOST )
	{
		
		
	}
	else if( action == SHOOT )
	{
		if( frameTest % 30 == 0 && frameTest < 30 * 5 )
		{
			
			launcher->position = position;
			//V2d test = normalize( V2d(  fRand( -1, 1 ),  fRand( -1, 1 ) ) );
			//can shoot in 1 of 32 (or w.e.) directions. can't shoot
			//the same direction twice
			V2d gAlong = normalize( mover->ground->v1 - mover->ground->v0 );
			double angle = atan2( gAlong.y, gAlong.x );
			angle -= PI / 16.0 * bulletDirIndex[shootIndex];
			//angle = 0;
			//angle -= PI / 2;
			
			V2d dir( cos( angle ), sin( angle ) );
			cout << "shooting: " << shootIndex 
				<< ", dir: " << bulletDirIndex[shootIndex] << endl;
			launcher->facingDir = dir;
			launcher->Fire();
			++shootIndex;
		}
		mover->SetSpeed( 0 );
		frameTest++;
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
}

void Boss_Crawler::UpdatePhysics()
{
	launcher->UpdatePhysics();
	specterProtected = false;

	if( dead )
	{
		return;
	}



//	double f = moveBezTest.GetValue( bezFrame / (double)bezLength );
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

	/*if( mover->ground != NULL )
	{
	}
	else
	{
		mover->velocity += gravity / (NUM_STEPS * slowMultiple);

		if( mover->velocity.y >= maxFallSpeed )
		{
			mover->velocity.y = maxFallSpeed;
		}
	}*/

	
	//mover->groundSpeed = 5;

	
	/*double beforeDistance;
	if( mover->groundSpeed > 0 )
	{
		beforeDistance = GetDistanceClockwise( travelIndex );
	}
	else if( mover->groundSpeed < 0 )
	{
		beforeDistance = GetDistanceCCW( travelIndex );
	}*/

	mover->Move( slowMultiple );

	
	

	Edge *bEdge = bulletHits[travelIndex].edge;
	double bq = bulletHits[travelIndex].quantity;
	bool align = false;

	if( !leftFirstEdge )
	{	
		if( firstEdge != bEdge )
		{
			if( mover->ground != firstEdge )
				leftFirstEdge = true;
		}
		else
		{
			
			if( mover->groundSpeed > 0 && mover->ground->edge1 == bEdge )
			{
				leftFirstEdge = true;
			}
			else if( mover->groundSpeed < 0 && mover->ground->edge0 == bEdge )
			{
				leftFirstEdge = true;
			}
		}
	}


	if( leftFirstEdge )
	{
		if( mover->groundSpeed > 0 )
		{
			if( ( mover->ground == bEdge && mover->edgeQuantity > bq )
				|| ( mover->ground->edge0 == bEdge ) )
			{
				align = true;
			}
		}
		else if( mover->groundSpeed < 0 )
		{
			if( ( mover->ground == bEdge && mover->edgeQuantity < bq )
				|| ( mover->ground->edge1 == bEdge ) )
			{
				align = true;
			}
		}
	}

	if( align )
	{
		/*mover->ground = bEdge;
		mover->edgeQuantity = bq;
		mover->roll = false;
		mover->UpdateGroundPos();*/

		if( travelIndex == numBullets - 1 )
		{
			cout << "setting to shoot!. travelindex: " << travelIndex << endl;
			for( int i = 0; i < 5 * 4; ++i )
			{
				markerVA[i].position = Vector2f( 0, 0 );
			}

			//init
			// 1-14
			SetDirs();

			action = SHOOT;

			shootIndex = 0;

			bulletIndex = 0;
			frameTest = 0;
			mover->SetSpeed( 0 );
		}
		else
		{
			
			travelIndex++;
			cout << "new travelindex: " << travelIndex << endl;

			//action = BOOST;
			//frameTest = 0;
			leftFirstEdge = false;
			firstEdge = mover->ground;

			if( GetClockwise( travelIndex ) )
			{
				mover->SetSpeed( 20 + travelIndex * 8 );
			}
			else
			{
				mover->SetSpeed( -20 - travelIndex * 8 );
			}
		}
		
	}
	/*if( mover->groundSpeed > 0 )
	{
		if( GetDistanceClockwise( travelIndex ) > beforeDistance )
		{
			mover->ground = bulletHits[travelIndex].edge;
			mover->edgeQuantity = bulletHits[travelIndex].quantity;
			mover->roll = false;
			mover->UpdateGroundPos();
			action = SHOOT;
			frameTest = 0;
			mover->SetSpeed( 0 );
		}
	}
	else if( mover->groundSpeed < 0 )
	{
		if( GetDistanceCCW( travelIndex ) > beforeDistance )
		{
			mover->ground = bulletHits[travelIndex].edge;
			mover->edgeQuantity = bulletHits[travelIndex].quantity;
			mover->roll = false;
			mover->UpdateGroundPos();
			action = SHOOT;
			frameTest = 0;
			mover->SetSpeed( 0 );
			
		}
	}*/

	position = mover->physBody.globalPosition;
	
	PhysicsResponse();
}

bool Boss_Crawler::ResolvePhysics( V2d vel )
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

void Boss_Crawler::PhysicsResponse()
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
				owner->player->ConfirmHit( COLOR_BLUE, 5, .8, 2 * 6 * 3 );

				if( owner->player->ground == NULL && owner->player->velocity.y > 0 )
				{
					owner->player->velocity.y = 4;//.5;
				}

															//cout << "frame: " << owner->player->frame << endl;

			//owner->player->frame--;
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

void Boss_Crawler::UpdatePostPhysics()
{
	launcher->UpdatePostPhysics();

	if( receivedHit != NULL )
		owner->Pause( 5 );

	if( deathFrame == 30 )
	{
		owner->RemoveEnemy( this );
		return;
	}

	

	UpdateSprite();
	launcher->UpdateSprites();

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

bool Boss_Crawler::PlayerSlowingMe()
{
	Actor *player = owner->player;
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

void Boss_Crawler::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		target->draw( markerVA );
		target->draw( sprite );
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

void Boss_Crawler::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape cs;
	cs.setRadius( 50 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );

	/*if( hasMonitor && !suppressMonitor )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}*/
}

bool Boss_Crawler::IHitPlayer()
{
	Actor *player = owner->player;
	
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

 pair<bool, bool> Boss_Crawler::PlayerHitMe()
{
	Actor *player = owner->player;

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

void Boss_Crawler::UpdateSprite()
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

void Boss_Crawler::DebugDraw( RenderTarget *target )
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

void Boss_Crawler::SaveEnemyState()
{
}

void Boss_Crawler::LoadEnemyState()
{
}

void Boss_Crawler::HitTerrain( double &q )
{
	//useless
}

bool Boss_Crawler::StartRoll()
{

}

void Boss_Crawler::FinishedRoll()
{

}

void Boss_Crawler::BulletHitTerrain( BasicBullet *b,
		Edge *edge, sf::Vector2<double> &pos )
{	
	cout << "bullet index: " << bulletIndex << endl;
	int size = 25;
	markerVA[bulletIndex*4+0].position = Vector2f( pos.x - size, pos.y - size );
	markerVA[bulletIndex*4+1].position = Vector2f( pos.x + size, pos.y - size );
	markerVA[bulletIndex*4+2].position = Vector2f( pos.x + size, pos.y + size );
	markerVA[bulletIndex*4+3].position = Vector2f( pos.x - size, pos.y + size );

	bulletHits[bulletIndex].edge = edge;
	bulletHits[bulletIndex].quantity = edge->GetQuantity( pos );
	b->launcher->DeactivateBullet( b );
	bulletIndex++;
}

void Boss_Crawler::BulletHitPlayer( BasicBullet *b )
{
}

void Boss_Crawler::HitTerrainAerial(Edge *edge, double q)
{
}

void Boss_Crawler::TransferEdge( Edge *edge )
{
	/*if( edge == bulletHits[travelIndex].edge )
	{
		onTargetEdge = true;
		
	}*/
}