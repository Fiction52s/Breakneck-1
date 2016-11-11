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

StagBeetle::StagBeetle( GameSession *owner, bool p_hasMonitor, Edge *g, double q, bool cw, double s )
	:Enemy( owner, EnemyType::STAGBEETLE, p_hasMonitor, 2 ), facingRight( cw ),
	moveBezTest( .22,.85,.3,.91 )
{

	

	gravity = V2d( 0, .6 );
	maxGroundSpeed = s;
	action = IDLE;
	initHealth = 60;
	health = initHealth;
	dead = false;
	deathFrame = 0;

	maxFallSpeed = 25;

	//ts_walk = owner->GetTileset( "crawlerwalk.png", 96, 64 );
	//ts_roll = owner->GetTileset( "crawlerroll.png", 96, 64 );

	attackFrame = -1;
	attackMult = 3;

	double height = 128;
	double width = 128;

	startGround = g;
	startQuant = q;
	frame = 0;

	testMover = new GroundMover( owner, g, q, 40, true, this );
	//testMover->gravity = V2d( 0, .5 );
	testMover->SetSpeed( 0 );
	//testMover->groundSpeed =   b s;
	/*if( !facingRight )
	{
		testMover->groundSpeed = -testMover->groundSpeed;
	}*/

	ts_death = owner->GetTileset( "stag_death_192x144.png", 192, 144 );
	//actionLength[DEATH] = 1;

	ts_hop = owner->GetTileset( "stag_hop_192x144.png", 192, 144 );
	ts_idle = owner->GetTileset( "stag_idle_192x144.png", 192, 144 );
	actionLength[IDLE] = 11 * 5;

	ts_run = owner->GetTileset( "stag_run_192x144.png", 192, 144 );
	actionLength[RUN] = 9 * 4;
	ts_sweep = owner->GetTileset( "stag_sweep_192x208.png", 192, 208 );

	actionLength[LAND] = 1;

	actionLength[JUMP] = 2;

	ts_walk = owner->GetTileset( "stag_walk_192x144.png", 192, 144 );

	

	//ts = owner->GetTileset( "crawler_128x128.png", width, height );
	sprite.setTexture( *ts_idle->texture );
	sprite.setTextureRect( ts_idle->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = g->GetPoint( q );
	V2d gNorm = g->Normal();
	sprite.setPosition( gPoint.x, gPoint.y );
	double angle = atan2( gNorm.x, -gNorm.y );
	sprite.setRotation( angle / PI * 180.f );
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
	hitboxInfo->damage = 18;
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

	//ts_testBlood = owner->GetTileset( "blood1.png", 32, 48 );
	//ts_testBlood = owner->GetTileset( "fx_blood_2_256x256.png", 256, 256 );
	//bloodSprite.setTexture( *ts_testBlood->texture );

	

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
	action = IDLE;
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
	//angle = atan2( gn.x, -gn.y );
	deathFrame = 0;
	dead = false;


	sprite.setTexture( *ts_idle->texture );
	sprite.setTextureRect( ts_idle->GetSubRect( 0 ) );
	//----update the sprite
	//double angle = 0;
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
	if( frame == actionLength[action] )
	{
		switch( action )
		{
		case IDLE:
			frame = 0;
			break;
		case RUN:
			frame = 0;
			break;
		case JUMP:
			frame = 1;
			break;
		case LAND:
			cout << "LANDING INTO RUN" << endl;
			//sprite.setTexture( *ts_run->texture );
			action = RUN;
			frame = 0;
			break;
		}
	}
}

void StagBeetle::UpdatePrePhysics()
{
	//testLaunch->UpdatePrePhysics();
	Actor *player = owner->player;

	if( dead )
		return;

	ActionEnded();

	if( attackFrame == 11 * attackMult )
	{
		attackFrame = -1;
		/*switch( action )
		{
		case IDLE:
			sprite.setTexture( *ts_idle->texture );
			break;
		case RUN:
			sprite.setTexture( *ts_run->texture );
			break;
		case JUMP:
			sprite.setTexture( *ts_hop->texture );
			break;
		case LAND:
			sprite.setTexture( *ts_hop->texture );
			break;
		}*/
	}

	switch( action )
	{
	case IDLE:
		{
			double dist = length( owner->player->position - position );
			if( dist < 800 )
			{
				action = RUN;
				frame = 0;
				sprite.setTexture( *ts_run->texture );
			}
		}
		break;
	case RUN:
		{
			double dist = length( owner->player->position - position );
			if( dist >= 900 )
			{
				action = IDLE;
				frame = 0;
				sprite.setTexture( *ts_idle->texture );
			}
		}
		break;
	case JUMP:
		break;
	//case ATTACK:
	//	break;
	case LAND:
		break;
	}

	switch( action )
	{
	case IDLE:
		testMover->SetSpeed( 0 );
		//cout << "idle: " << frame << endl;
		break;
	case RUN:
		//cout << "run: " << frame << endl;
		if( facingRight )
		{
			if( player->position.x < position.x )
			{
				facingRight = false;
			}
		}
		else
		{
			if( player->position.x > position.x )
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
		//cout << "jump: " << frame << endl;
		break;
//	case ATTACK:
	//	{
	//		testMover->SetSpeed( 0 );
	//	}
	//	break;
	case LAND:
		{
		//	cout << "land: " << frame << endl;
			//testMover->SetSpeed( 0 );
		}
		break;
	default:
		//cout << "WAATATET" << endl;
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
			if( hasMonitor && !suppressMonitor )
				owner->keyMarker->CollectKey();
			//AttemptSpawnMonitor();
			dead = true;
			owner->player->ConfirmEnemyKill( this );
		}
		else
		{
			owner->player->ConfirmEnemyNoKill( this );
		}

		receivedHit = NULL;
	}

	
}

void StagBeetle::UpdatePhysics()
{
	//testLaunch->UpdatePhysics();
	specterProtected = false;

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
		angle = 0;
		bool roll = testMover->roll;
		//double angle = 0;
		Edge *ground = testMover->ground;
		double edgeQuantity = testMover->edgeQuantity;

		if( ground != NULL )
		{
			V2d gn = ground->Normal();
			//double angle;
			if( roll )
			{
				if( facingRight )
				{
					V2d vec = normalize( position - ground->v1 );
					angle = atan2( vec.y, vec.x );
					angle += PI / 2.0;
				}
				else
				{
					V2d vec = normalize( position - ground->v0 );
					angle = atan2( vec.y, vec.x );
					angle += PI / 2.0;
				}	
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}
		}
		else
		{
			angle = 0;
		}

		angle = angle * 180 / PI;

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

				owner->player->ConfirmHit( 2, 5, .8, 6 );

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
		t.rotate( angle );
		Vector2f newPoint = t.transformPoint( Vector2f( 1, -1 ) );
		deathVector = V2d( newPoint.x, newPoint.y );

		//queryMode = "reverse";

		//physbody is a circle
		//Rect<double> r( position.x - physBody.rw, position.y - physBody.rw, physBody.rw * 2, physBody.rw * 2 );
		//owner->crawlerReverserTree->Query( this, r );
	}
}

void StagBeetle::UpdatePostPhysics()
{
	if( deathFrame == 30 )
	{
		owner->RemoveEnemy( this );
		return;
	}

	if( receivedHit != NULL )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_hitSpack, ( owner->player->position + position ) / 2.0, true, 0, 10, 2, true );
		owner->Pause( 5 );
	}

	

	if( deathFrame == 0 && dead )
	{
		owner->ActivateEffect( EffectLayer::IN_FRONT, ts_blood, position, true, 0, 15, 2, true );
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

void StagBeetle::Draw(sf::RenderTarget *target )
{
	if( !dead )
	{
		//cout << "Drawing" << endl;
		if( hasMonitor && !suppressMonitor )
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite, keyShader );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			target->draw( *keySprite );
		}
		else
		{
			if( owner->pauseFrames < 2 || receivedHit == NULL )
			{
				target->draw( sprite );
			}
			else
			{
				target->draw( sprite, hurtShader );
			}
			
		}
	}
	else
	{
		target->draw( botDeathSprite );

		if( deathFrame / 3 < 6 )
		{
			
			/*bloodSprite.setTextureRect( ts_testBlood->GetSubRect( deathFrame / 3 ) );
			bloodSprite.setOrigin( bloodSprite.getLocalBounds().width / 2, bloodSprite.getLocalBounds().height / 2 );
			bloodSprite.setPosition( position.x, position.y );
			bloodSprite.setScale( 2, 2 );
			target->draw( bloodSprite );*/
		}
		
		target->draw( topDeathSprite );
	}
}

void StagBeetle::DrawMinimap( sf::RenderTarget *target )
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
}

bool StagBeetle::IHitPlayer()
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

 pair<bool, bool> StagBeetle::PlayerHitMe()
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

void StagBeetle::UpdateSprite()
{
	if( dead )
	{
		//cout << "deathVector: " << deathVector.x << ", " << deathVector.y << endl;
		botDeathSprite.setTexture( *ts_death->texture );
		botDeathSprite.setTextureRect( ts_death->GetSubRect( 0 ) );
		botDeathSprite.setOrigin( botDeathSprite.getLocalBounds().width / 2, botDeathSprite.getLocalBounds().height / 2);
		botDeathSprite.setPosition( position.x + deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + deathVector.y * deathPartingSpeed * deathFrame );
		botDeathSprite.setRotation( sprite.getRotation() );

		topDeathSprite.setTexture( *ts_death->texture );
		topDeathSprite.setTextureRect( ts_death->GetSubRect( 1 ) );
		topDeathSprite.setOrigin( topDeathSprite.getLocalBounds().width / 2, topDeathSprite.getLocalBounds().height / 2 );
		topDeathSprite.setPosition( position.x + -deathVector.x * deathPartingSpeed * deathFrame, 
			position.y + -deathVector.y * deathPartingSpeed * deathFrame );
		topDeathSprite.setRotation( sprite.getRotation() );
	}
	else
	{
		Edge *ground = testMover->ground;
		double edgeQuantity = testMover->edgeQuantity;

		V2d gPoint;
		if( ground != NULL )
		{
			gPoint = position;//ground->GetPoint( edgeQuantity );
		}
		else
		{

			gPoint = position;
		}

		IntRect r;
		int originHeight = 144 - ( 48 );
		int attackOriginHeight = ts_sweep->tileHeight - ( 48 );

		if( attackFrame >= 0 )
		{
			sprite.setTexture( *ts_sweep->texture );
			r = ts_sweep->GetSubRect( attackFrame / attackMult );
			if( facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, attackOriginHeight);
			sprite.setRotation( angle );
			sprite.setPosition( gPoint.x, gPoint.y );
			//cout << "attacking angle " << angle << endl;
		}
		else
		{
			switch( action )
			{
			case IDLE:
			//	cout << "idle angle: " << angle << endl;
				//sprite.setTextureRect( *ts_idle->texture );
				sprite.setTexture( *ts_idle->texture );
				r = ts_idle->GetSubRect( frame / 5 );

				if( facingRight )
				{
					r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
				}
				sprite.setTextureRect( r );

				sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight);
				sprite.setRotation( angle );
				sprite.setPosition( gPoint.x, gPoint.y );
				break;
			case RUN:
				//cout << "run angle: " << angle << endl;
				//sprite.setTexture( *ts_run->texture );
				sprite.setTexture( *ts_run->texture );
				r = ts_run->GetSubRect( frame / 4 );

				if( facingRight )
				{
					r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
				}
				sprite.setTextureRect( r );

				sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight);
				sprite.setRotation( angle );
				sprite.setPosition( gPoint.x, gPoint.y );
				break;
			case JUMP:
				{
					int tFrame = 0;
				sprite.setTexture( *ts_hop->texture );
				//cout << "jump angle: " << angle << endl;

				if( frame == 0 )
				{

				}
				else
				{
					if( testMover->velocity.y < 0 )
					{
						tFrame = 1;
					}
					else
						tFrame = 2;
				}
				
				r = ts_hop->GetSubRect( tFrame );
				if( facingRight )
				{
					r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
				}
				sprite.setTextureRect( r );
				sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight );
				/*if( tFrame > 1 )
				{
					sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
				}
				else
				{
					
				}*/
				
				sprite.setPosition( position.x, position.y );
				sprite.setRotation( 0 );
				break;
				}
			case LAND:
				sprite.setTexture( *ts_hop->texture );
				r = ts_hop->GetSubRect( 3 );

				if( facingRight )
				{
					r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
				}
				sprite.setTextureRect( r );

				sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight);
				sprite.setRotation( angle );
				sprite.setPosition( gPoint.x, gPoint.y );
			//	cout << "land angle: " << angle << endl;
				
				break;
			}
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
	//cout << "hit terrain? " << endl;
}

bool StagBeetle::StartRoll()
{

}

void StagBeetle::FinishedRoll()
{

}

void StagBeetle::HitOther()
{
	//cout << "hit other" << endl;
	
	if( action == RUN )
	{
	if( (facingRight && testMover->groundSpeed < 0 )
		|| ( !facingRight && testMover->groundSpeed > 0 ) )
	{
		cout << "here" << endl;
		testMover->SetSpeed( 0 );
	}
	else if( facingRight && testMover->groundSpeed > 0 )
	{
		V2d v = V2d( maxGroundSpeed, -10 );
		testMover->Jump( v );
		action = JUMP;
		frame = 0;
	}
	else if( !facingRight && testMover->groundSpeed < 0 )
	{
		V2d v = V2d( -maxGroundSpeed, -10 );
		testMover->Jump( v );
		action = JUMP;
		frame = 0;
	}
	}
	
	//cout << "hit other!" << endl;
	//testMover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void StagBeetle::ReachCliff()
{
	if( (facingRight && testMover->groundSpeed < 0 )
		|| (!facingRight && testMover->groundSpeed > 0) )
	{
		testMover->SetSpeed( 0 );
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;

	V2d v;
	if( facingRight )
	{
		v = V2d( maxGroundSpeed, -10 );
	}
	else
	{
		v = V2d( -maxGroundSpeed, -10 );
	}

	testMover->Jump( v );

	action = JUMP;
	frame = 0;

	//cout << "Reach cliff" << endl;
	//testMover->groundSpeed = -testMover->groundSpeed;
	//facingRight = !facingRight;
}

void StagBeetle::HitOtherAerial( Edge *e )
{
	//cout << "hit edge" << endl;
}

void StagBeetle::Land()
{
	action = LAND;
	//sprite.setTexture( *ts_hop->texture );
	//sprite.setTextureRect( ts_hop->GetSubRect( 0 ) );
	frame = 0;
	cout << "land" << endl;
}