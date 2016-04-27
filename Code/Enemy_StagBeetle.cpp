#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>

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

StagBeetle::StagBeetle( GameSession *owner, Edge *g, double q, bool cw, double s )
	:Enemy( owner, EnemyType::STAGBEETLE ), facingRight( cw ),
	moveBezTest( .22,.85,.3,.91 )
{
	gravity = V2d( 0, .6 );
	maxGroundSpeed = s;
	action = RUN;
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

	testMover = new GroundMover( owner, g, q, 32, true, this );
	//testMover->gravity = V2d( 0, .5 );
	testMover->SetSpeed( 0 );
	//testMover->groundSpeed = s;
	/*if( !facingRight )
	{
		testMover->groundSpeed = -testMover->groundSpeed;
	}*/

	ts = owner->GetTileset( "crawler_128x128.png", width, height );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = g->GetPoint( q );
	sprite.setPosition( testMover->physBody.globalPosition.x,
		testMover->physBody.globalPosition.y );
	position = testMover->physBody.globalPosition;
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
	hitboxInfo->damage = 100;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;


	/*testLaunch = new Launcher( this, owner, 10, 1,
		testMover->physBody.globalPosition, g->Normal(), 0 );*/
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

	testMover->SetSpeed( 0 );
	//testMover->Move( slowMultiple );

	//ground = testMover->ground;
	//edgeQuantity = testMover->edgeQuantity;
	//position = testMover->physBody.globalPosition;
}

void StagBeetle::ResetEnemy()
{
	testMover->ground = startGround;
	testMover->edgeQuantity = startQuant;
	testMover->roll = false;
	testMover->UpdateGroundPos();
	testMover->SetSpeed( 0 );

	position = testMover->physBody.globalPosition;
	//testMover->UpdateGroundPos();

	//testLaunch->Reset();
	//testLaunch->position = testMover->physBody.globalPosition;
	//testLaunch->facingDir = startGround->Normal();

	bezFrame = 0;
	health = initHealth;
	attackFrame = -1;
	//lastReverser = false;
	//roll = false;
	//ground = startGround;
	//edgeQuantity = startQuant;
	V2d gPoint = testMover->ground->GetPoint( testMover->edgeQuantity );
	//sprite.setPosition( testMover->physBody.globalPosition.x,
	//	testMover->physBody.globalPosition.y );
	frame = 0;

	V2d gn = testMover->ground->Normal();
	//testMover->physBody.globalPosition = gPoint + testMover->ground->Normal() * 64.0 / 2.0;

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

int StagBeetle::NumTotalBullets()
{
	return 0;
}

void StagBeetle::HandleEntrant( QuadTreeEntrant *qte )
{
	assert( queryMode != "" );

	//might need for other queries but def not for physics
}

void StagBeetle::UpdateHitboxes()
{
	Edge *ground = testMover->ground;
	if( ground != NULL )
	{
		//V2d gn = ground->Normal();
		//double angle = 0;
		
		
		//angle = atan2( gn.x, -gn.y );
		
		//hitBody.globalAngle = angle;
		//hurtBody.globalAngle = angle;

		V2d knockbackDir( 1, -1 );
		knockbackDir = normalize( knockbackDir );
		if( testMover->groundSpeed > 0 )
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
	hitBody.globalPosition = testMover->physBody.globalPosition;
	hurtBody.globalPosition = testMover->physBody.globalPosition;
	//physBody.globalPosition = position;//+ V2d( -16, 0 );// + //physBody.offset + offset;
}

void StagBeetle::ActionEnded()
{
	switch( action )
	{
	case RUN:
		frame = 0;
		break;
	case JUMP:
		frame = 0;
		break;
	case LAND:
		action = RUN;
		frame = 0;
		break;
	case ATTACK:
		action = RUN;
		frame = 0;
		break;
	}
}

void StagBeetle::UpdatePrePhysics()
{
	//testLaunch->UpdatePrePhysics();
	Actor &player = owner->player;

	if( dead )
		return;

	ActionEnded();

	switch( action )
	{
	case RUN:
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
	case RUN:
		if( facingRight )
		{
			if( player.position.x < position.x )
			{
				facingRight = false;
			}
		}
		else
		{
			if( player.position.x > position.x )
			{
				facingRight = true;
			}
		}

		if( facingRight )
		{
			testMover->SetSpeed( testMover->groundSpeed + .3 );
		}
		else
		{
			testMover->SetSpeed( testMover->groundSpeed - .3 );
		}

		if( testMover->groundSpeed > maxGroundSpeed )
			testMover->SetSpeed( maxGroundSpeed );
		else if( testMover->groundSpeed < -maxGroundSpeed )
			testMover->SetSpeed( -maxGroundSpeed );
		break;
	case JUMP:
		break;
	case ATTACK:
		{
			testMover->SetSpeed( 0 );
		}
		break;
	case LAND:
		{
			testMover->SetSpeed( 0 );
		}
		break;
	}


	bool roll = testMover->roll;

	if( !dead && receivedHit != NULL )
	{	
		//gotta factor in getting hit by a clone
		health -= 20;

		//cout << "health now: " << health << endl;

		if( health <= 0 )
		{
			AttemptSpawnMonitor();
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

		if( !roll && frame == 16 * crawlAnimationFactor )
		{
			frame = 0;
		}

		if ( roll && frame == 10 * rollAnimationFactor )
		{
			frame = rollAnimationFactor * 2; 
		}

		//cout << "groundspeed: " << testMover->groundSpeed << endl;
	//}

	/*if( bezFrame == 0 )
	{
		testLaunch->position = position;
		if( testMover->ground != NULL )
		{
			testLaunch->facingDir = testMover->ground->Normal();
		}
		else
		{
			testLaunch->facingDir = V2d( 0, -1 );
		}
		
		testLaunch->Fire();
	}*/
}

void StagBeetle::UpdatePhysics()
{
	//testLaunch->UpdatePhysics();


	if( dead )
	{
		return;
	}



	double f = moveBezTest.GetValue( bezFrame / (double)bezLength );
	//testMover->groundSpeed = groundSpeed;// * f;
	if( !facingRight )
	{
	//	testMover->groundSpeed = groundSpeed;// * f;
	}
	bezFrame++;

	if( bezFrame == bezLength )
	{
		bezFrame = 0;
		

	}

	if( testMover->ground != NULL )
	{
	}
	else
	{
		testMover->velocity += gravity / (NUM_STEPS * slowMultiple);

		if( testMover->velocity.y >= maxFallSpeed )
		{
			testMover->velocity.y = maxFallSpeed;
		}
	}

	
	//testMover->groundSpeed = 5;
	testMover->Move( slowMultiple );

	position = testMover->physBody.globalPosition;
	
	PhysicsResponse();
}

bool StagBeetle::ResolvePhysics( V2d vel )
{
	possibleEdgeCount = 0;

	double rw = testMover->physBody.rw;
	double rh = testMover->physBody.rh;

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

void StagBeetle::PhysicsResponse()
{
	if( !dead  )
	{
		bool roll = testMover->roll;
		double angle = 0;
		Edge *ground = testMover->ground;
		double edgeQuantity = testMover->edgeQuantity;

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
			
			//V2d pp = ground->GetPoint( testMover->edgeQuantity );
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
			V2d p = testMover->physBody.globalPosition;

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
				owner->player.test = true;
				owner->player.currAttackHit = true;
				owner->player.flashColor = COLOR_BLUE;
				owner->player.flashFrames = 5;
				owner->player.currentSpeedBar += .8;
				owner->player.swordShader.setParameter( "energyColor", COLOR_BLUE );
				owner->player.desperationMode = false;
				owner->powerBar.Charge( 2 * 6 * 2 );

				if( owner->player.ground == NULL && owner->player.velocity.y > 0 )
				{
					owner->player.velocity.y = 4;//.5;
				}

															//cout << "frame: " << owner->player.frame << endl;

			//owner->player.frame--;
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

void StagBeetle::UpdatePostPhysics()
{
	if( receivedHit != NULL )
		owner->Pause( 5 );

	if( deathFrame == 30 )
	{
		owner->RemoveEnemy( this );
		return;
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

bool StagBeetle::PlayerSlowingMe()
{
	Actor &player = owner->player;
	for( int i = 0; i < player.maxBubbles; ++i )
	{
		if( player.bubbleFramesToLive[i] > 0 )
		{
			if( length( position - player.bubblePos[i] ) <= player.bubbleRadius )
			{
				return true;
			}
		}
	}
	return false;
}

void StagBeetle::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		if( monitor != NULL )
		{
			//owner->AddEnemy( monitor );
			CircleShape cs;
			cs.setRadius( 55 );
			switch( monitor->monitorType )
			{
			case Monitor::BLUE:
				cs.setFillColor( COLOR_BLUE );
				break;
			case Monitor::GREEN:
				cs.setFillColor( COLOR_GREEN );
				break;
			case Monitor::YELLOW:
				cs.setFillColor( COLOR_YELLOW );
				break;
			case Monitor::ORANGE:
				cs.setFillColor( COLOR_ORANGE );
				break;
			case Monitor::RED:
				cs.setFillColor( COLOR_RED );
				break;
			case Monitor::MAGENTA:
				cs.setFillColor( COLOR_MAGENTA );
				break;
			case Monitor::WHITE:
				cs.setFillColor( COLOR_WHITE );
				break;
			}
			
			cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
			cs.setPosition( position.x, position.y );
			target->draw( cs );
		}
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

void StagBeetle::DrawMinimap( sf::RenderTarget *target )
{
	CircleShape cs;
	cs.setRadius( 50 );
	cs.setFillColor( COLOR_BLUE );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( position.x, position.y );
	target->draw( cs );

	if( monitor != NULL )
	{
		monitor->miniSprite.setPosition( position.x, position.y );
		target->draw( monitor->miniSprite );
	}
}

bool StagBeetle::IHitPlayer()
{
	Actor &player = owner->player;
	
	if( player.invincibleFrames == 0 && hitBody.Intersects( player.hurtBody ) )
	{
		if( player.position.x < position.x )
		{
			hitboxInfo->kbDir.x = -abs( hitboxInfo->kbDir.x );
			//cout << "left" << endl;
		}
		else if( player.position.x > position.x )
		{
			//cout << "right" << endl;
			hitboxInfo->kbDir.x = abs( hitboxInfo->kbDir.x );
		}
		else
		{
			//dont change it
		}
		attackFrame = 0;
		player.ApplyHit( hitboxInfo );
		return true;
	}
	
	return false;
}

 pair<bool, bool> StagBeetle::PlayerHitMe()
{
	Actor &player = owner->player;

	if( player.currHitboxes != NULL )
	{
		bool hit = false;

		for( list<CollisionBox>::iterator it = player.currHitboxes->begin(); it != player.currHitboxes->end(); ++it )
		{
			if( hurtBody.Intersects( (*it) ) )
			{
				hit = true;
				break;
			}
		}
		

		if( hit )
		{
			receivedHit = player.currHitboxInfo;
			return pair<bool, bool>(true,false);
		}
		
	}

	for( int i = 0; i < player.recordedGhosts; ++i )
	{
		if( player.ghostFrame < player.ghosts[i]->totalRecorded )
		{
			if( player.ghosts[i]->currHitboxes != NULL )
			{
				bool hit = false;
				
				for( list<CollisionBox>::iterator it = player.ghosts[i]->currHitboxes->begin(); it != player.ghosts[i]->currHitboxes->end(); ++it )
				{
					if( hurtBody.Intersects( (*it) ) )
					{
						hit = true;
						break;
					}
				}
		

				if( hit )
				{
					receivedHit = player.currHitboxInfo;
					return pair<bool, bool>(true,true);
				}
			}
			//player.ghosts[i]->curhi
		}
	}
	return pair<bool, bool>(false,false);
}

void StagBeetle::UpdateSprite()
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

void StagBeetle::DebugDraw( RenderTarget *target )
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
		testMover->physBody.DebugDraw( target );
	}
//	hurtBody.DebugDraw( target );
//	hitBody.DebugDraw( target );
}

void StagBeetle::SaveEnemyState()
{
}

void StagBeetle::LoadEnemyState()
{
}

void StagBeetle::HitTerrain( double &q )
{
	
}

bool StagBeetle::StartRoll()
{

}

void StagBeetle::FinishedRoll()
{

}

void StagBeetle::HitOther()
{
	V2d v;
	if( facingRight && testMover->groundSpeed > 0 )
	{
		v = V2d( 10, -10 );
		testMover->Jump( v );
	}
	else if( !facingRight && testMover->groundSpeed < 0 )
	{
		v = V2d( -10, -10 );
		testMover->Jump( v );
	}
	//cout << "hit other!" << endl;
	//testMover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void StagBeetle::ReachCliff()
{
	if( facingRight && testMover->groundSpeed < 0 
		|| !facingRight && testMover->groundSpeed > 0 )
	{
		testMover->SetSpeed( 0 );
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

	testMover->Jump( v );
	//testMover->groundSpeed = -testMover->groundSpeed;
	//facingRight = !facingRight;
}

void StagBeetle::HitOtherAerial()
{
	//cout << "hit edge" << endl;
}

void StagBeetle::Land()
{
	frame = 0;
	//cout << "land" << endl;
}