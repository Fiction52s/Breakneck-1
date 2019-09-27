#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_StagBeetle.h"
#include "Shield.h"

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

StagBeetle::StagBeetle( GameSession *owner, bool p_hasMonitor, Edge *g, double q, int p_level )
	:Enemy( owner, EnemyType::EN_STAGBEETLE, p_hasMonitor, 2 ),
	moveBezTest( .22,.85,.3,.91 )
{
	level = p_level;

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

	facingRight = true;
	gravity = V2d( 0, .6 );
	maxGroundSpeed = 10;
	action = IDLE;
	dead = false;

	maxFallSpeed = 40;

	attackFrame = -1;
	attackMult = 3;

	//double height = 128 * scale;
	//double width = 128 * scale;

	startGround = g;
	startQuant = q;
	frame = 0;

	testMover = new GroundMover( owner, g, q, 40 * scale, true, this );
	testMover->AddAirForce(V2d(0, .5));
	//testMover-> = V2d( 0, .5 );
	testMover->SetSpeed( 0 );
	//testMover->groundSpeed =   b s;
	/*if( !facingRight )
	{
		testMover->groundSpeed = -testMover->groundSpeed;
	}*/

	ts_death = owner->GetTileset( "Enemies/stag_death_192x144.png", 192, 144 );
	//actionLength[DEATH] = 1;

	ts_hop = owner->GetTileset( "Enemies/stag_hop_192x144.png", 192, 144 );
	ts_idle = owner->GetTileset( "Enemies/stag_idle_192x144.png", 192, 144 );
	actionLength[IDLE] = 11 * 5;

	ts_run = owner->GetTileset( "Enemies/stag_run_192x144.png", 192, 144 );
	actionLength[RUN] = 9 * 4;
	ts_sweep = owner->GetTileset( "Enemies/stag_sweep_192x208.png", 192, 208 );

	actionLength[LAND] = 1;

	actionLength[JUMP] = 2;

	ts_walk = owner->GetTileset( "Enemies/stag_walk_192x144.png", 192, 144 );

	sprite.setTexture( *ts_idle->texture );
	sprite.setTextureRect( ts_idle->GetSubRect( 0 ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	V2d gPoint = g->GetPoint( q );
	V2d gNorm = g->Normal();
	sprite.setPosition( gPoint.x, gPoint.y );
	double angle = atan2( gNorm.x, -gNorm.y );
	sprite.setRotation( angle / PI * 180.f );
	sprite.setScale(scale, scale);
	position = testMover->physBody.globalPosition;


	receivedHit = NULL;

	double size = max( 192 * scale, 144 * scale);
	spawnRect = sf::Rect<double>( gPoint.x - size, gPoint.y - size, size * 2, size * 2 );

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	SetupBodies(1, 1);
	AddBasicHurtCircle(32);
	AddBasicHitCircle(32);
	hitBody->hitboxInfo = hitboxInfo;



	crawlAnimationFactor = 5;
	rollAnimationFactor = 5;

	bezFrame = 0;
	bezLength = 60 * NUM_STEPS;

	testMover->SetSpeed( 0 );

	cutObject->SetTileset(ts_death);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(1);
	cutObject->SetScale(scale);

	shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);
	shield->SetPosition(position);

	ResetEnemy();
}

void StagBeetle::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	//if (!dead)
		//testMover->physBody.DebugDraw(target);
}

void StagBeetle::ResetEnemy()
{

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	action = IDLE;
	testMover->ground = startGround;
	testMover->edgeQuantity = startQuant;
	testMover->roll = false;
	testMover->UpdateGroundPos();
	testMover->SetSpeed( 0 );

	position = testMover->physBody.globalPosition;

	bezFrame = 0;
	attackFrame = -1;
	V2d gPoint = testMover->ground->GetPoint( testMover->edgeQuantity );
	frame = 0;

	V2d gn = testMover->ground->Normal();
	dead = false;

	angle = atan2( gn.x, -gn.y );
	receivedHit = NULL;

	UpdateSprite();
	UpdateHitboxes();

	currShield = shield;
	shield->Reset();
	shield->SetPosition(position);
}

void StagBeetle::UpdateHitboxes()
{
	Edge *ground = testMover->ground;
	if( ground != NULL )
	{

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

	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	hitBox.globalPosition = testMover->physBody.globalPosition;
	hurtBox.globalPosition = testMover->physBody.globalPosition;
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
			//cout << "LANDING INTO RUN" << endl;
			//sprite.setTexture( *ts_run->texture );
			action = RUN;
			frame = 0;
			break;
		}
	}
}

void StagBeetle::ProcessState()
{
	//cout << "vel: " << testMover->velocity.x << ", " << testMover->velocity.y << endl;
	Actor *player = owner->GetPlayer( 0 );

	ActionEnded();

	if( attackFrame == 11 * attackMult )
	{
		attackFrame = -1;
	}

	switch( action )
	{
	case IDLE:
		{
			double dist = length( owner->GetPlayer( 0 )->position - position );
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
			double dist = length( owner->GetPlayer( 0 )->position - position );
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
}

void StagBeetle::UpdateEnemyPhysics()
{
	if (numHealth > 0) //!dead
	{
		testMover->Move(slowMultiple, numPhysSteps);

		if (testMover->ground == NULL)
		{
			if (testMover->velocity.y > maxFallSpeed)
			{
				testMover->velocity.y = maxFallSpeed;
			}
		}

		position = testMover->physBody.globalPosition;

		shield->SetPosition(position);
	}



	//double f = moveBezTest.GetValue( bezFrame / (double)bezLength );
	////testMover->groundSpeed = groundSpeed;// * f;
	//if( !facingRight )
	//{
	////	testMover->groundSpeed = groundSpeed;// * f;
	//}
	//bezFrame++;

	//if( bezFrame == bezLength )
	//{
	//	bezFrame = 0;
	//	

	//}

	//if( testMover->ground != NULL )
	//{
	//}
	//else
	//{
	//	testMover->velocity += gravity / (NUM_STEPS * slowMultiple);

	//	if( testMover->velocity.y >= maxFallSpeed )
	//	{
	//		testMover->velocity.y = maxFallSpeed;
	//	}
	//}

	//
	////testMover->groundSpeed = 5;
	//testMover->Move( slowMultiple );

	//position = testMover->physBody.globalPosition;
	//
	//PhysicsResponse();
}





void StagBeetle::EnemyDraw(sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
}



void StagBeetle::UpdateSprite()
{
	Edge *ground = testMover->ground;
	double edgeQuantity = testMover->edgeQuantity;
	V2d gn;

	V2d gPoint;
	if( ground != NULL )
	{
		gPoint = position;//ground->GetPoint( edgeQuantity );
		gn = ground->Normal();
	}
	else
	{

		gPoint = position;
	}

	IntRect r;
	int originHeight = 144 - ( 48 );
	int attackOriginHeight = ts_sweep->tileHeight - ( 48 );

	//if( attackFrame >= 0 )
	//{
	//	sprite.setTexture( *ts_sweep->texture );
	//	r = ts_sweep->GetSubRect( attackFrame / attackMult );
	//	if( facingRight )
	//	{
	//		r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
	//	}
	//	sprite.setTextureRect( r );
	//	sprite.setOrigin( sprite.getLocalBounds().width / 2, attackOriginHeight);
	//	sprite.setRotation( angle );
	//	sprite.setPosition( gPoint.x, gPoint.y );
	//	//cout << "attacking angle " << angle << endl;
	//}
	//else
	{
		switch( action )
		{
		case IDLE:
		//	cout << "idle angle: " << angle << endl;
			//sprite.setTextureRect( *ts_idle->texture );
			sprite.setTexture( *ts_idle->texture );
			r = ts_idle->GetSubRect( frame / 5 );

			/*if( facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );

			sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight);
			sprite.setRotation( angle );
			sprite.setPosition( gPoint.x, gPoint.y );*/
			break;
		case RUN:
			//cout << "run angle: " << angle << endl;
			//sprite.setTexture( *ts_run->texture );
			sprite.setTexture( *ts_run->texture );
			r = ts_run->GetSubRect( frame / 4 );

			/*if( facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );

			sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight);
			sprite.setRotation( angle );
			sprite.setPosition( gPoint.x, gPoint.y );*/
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
			/*if( facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight );
				
			sprite.setPosition( position.x, position.y );
			sprite.setRotation( 0 );*/
			break;
			}
		case LAND:
			sprite.setTexture( *ts_hop->texture );
			r = ts_hop->GetSubRect( 3 );

			/*if( facingRight )
			{
				r = sf::IntRect( r.left + r.width, r.top, -r.width, r.height );
			}
			sprite.setTextureRect( r );

			sprite.setOrigin( sprite.getLocalBounds().width / 2, originHeight);
			sprite.setRotation( angle );
			sprite.setPosition( gPoint.x, gPoint.y );*/
		//	cout << "land angle: " << angle << endl;
				
			break;
		}

		if (facingRight)
		{
			r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
		}
		sprite.setTextureRect(r);

		float extraVert = 0;
		if (testMover->ground != NULL)
		{
			if (!testMover->roll)
			{
				angle = atan2(gn.x, -gn.y);

				V2d pp = testMover->ground->GetPoint(testMover->edgeQuantity);//ground->GetPoint( edgeQuantity );
				sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
				sprite.setRotation(angle / PI * 180);
				sprite.setPosition(pp.x, pp.y);
			}
			else
			{
				if (testMover->groundSpeed > 0 )//facingRight)
				{
					V2d vec = normalize(position - testMover->ground->v1);
					angle = atan2(vec.y, vec.x);
					angle += PI / 2.0;

					sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
					sprite.setRotation(angle / PI * 180);
					V2d pp = testMover->ground->GetPoint(testMover->edgeQuantity);//ground->GetPoint( edgeQuantity );
					sprite.setPosition(pp.x, pp.y);
				}
				else if( testMover->groundSpeed < 0 )
				{
					V2d vec = normalize(position - testMover->ground->v0);
					angle = atan2(vec.y, vec.x);
					angle += PI / 2.0;

					sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
					sprite.setRotation(angle / PI * 180);
					V2d pp = testMover->ground->GetPoint(testMover->edgeQuantity);
					sprite.setPosition(pp.x, pp.y);
				}
			}
		}
		else
		{
			sprite.setOrigin(sprite.getLocalBounds().width / 2, originHeight);
			sprite.setRotation(0);
			sprite.setPosition(gPoint.x, gPoint.y);
		}
	}
}

void StagBeetle::HitTerrain( double &q )
{
	//cout << "hit terrain? " << endl;
}

bool StagBeetle::StartRoll()
{
	return false;
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
	//cout << "land" << endl;
}