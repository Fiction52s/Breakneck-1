#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include <cstdlib>
#include "Enemy_PoisonFrog.h"

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



PoisonFrog::PoisonFrog( GameSession *p_owner, bool p_hasMonitor, Edge *g, double q, int p_level )
	:Enemy( p_owner, EnemyType::EN_POISONFROG, p_hasMonitor, 2 )
{
	gravityFactor = 30;
	jumpFramesWait = 60;
	jumpStrength = V2d(5, 12);


	level = p_level;
	maxFallSpeed = 25;
	actionLength[STAND] = 10;
	actionLength[JUMPSQUAT] = 2;
	actionLength[JUMP] = 2;
	actionLength[LAND] = 2;
	actionLength[STEEPJUMP] = 2;
	actionLength[WALLCLING] = 30;

	animFactor[STAND] = 1;
	animFactor[JUMPSQUAT] = 4;
	animFactor[JUMP] = 1;
	animFactor[LAND] = 3;
	animFactor[STEEPJUMP] = 1;
	animFactor[WALLCLING] = 1;

	invincibleFrames = 0;
	double width = 80;
	double height = 80;
	ts_test = owner->GetTileset( "Enemies/frog_80x80.png", width, height );

	//jumpStrength = 10;
	xSpeed = 8;

	mover = new GroundMover( p_owner, g, q, 30, true, this );
	mover->SetSpeed( 0 );
	
	//cout << "creating the boss crawler" << endl;
	action = STAND;
	gravity = gravityFactor / 64.0;
	facingRight = false;
	receivedHit = NULL;

	
	dead = false;
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( 0 ) );
	//sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	V2d gPoint = g->GetPoint( q );

	sprite.setTexture( *ts_test->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );



	sprite.setPosition( gPoint.x, gPoint.y );


	double size = max( width * 5, height * 5 );
	spawnRect = sf::Rect<double>( gPoint.x - size / 2, gPoint.y - size / 2, size, size);
	//spawnRect = sf::Rect<double>( gPoint.x - 96 / 2, gPoint.y - 96/ 2, 96, 96 );

	hurtBody = new CollisionBody(1);
	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = 0;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 30;
	hurtBox.rh = 30;
	hurtBody->AddCollisionBox(0, hurtBox);

	hitBody = new CollisionBody(1);
	CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = 0;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 30;
	hitBox.rh = 30;
	hitBody->AddCollisionBox(0, hitBox);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	hitBody->hitboxInfo = hitboxInfo;

	startGround = g;
	startQuant = q;
	frame = 0;
	position = mover->physBody.globalPosition;//gPoint + ground->Normal() * physBody.rh; //16.0;

	cutObject->SetTileset(ts_test);
	cutObject->SetSubRectFront(10);
	cutObject->SetSubRectBack(9);

	ResetEnemy();
}

void PoisonFrog::ResetEnemy()
{
	invincibleFrames = 0;
	//ground = startGround;
	//edgeQuantity = startQuant;
	hasDoubleJump = true;

	mover->ground = startGround;
	mover->edgeQuantity = startQuant;
	mover->roll = false;
	mover->UpdateGroundPos();
	mover->SetSpeed( 0 );

	position = mover->physBody.globalPosition;

	SetHurtboxes(hurtBody, 0);
	SetHitboxes(hitBody, 0);

	V2d gPoint = mover->ground->GetPoint( mover->edgeQuantity );

	sprite.setTexture( *ts_test->texture );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height );

	sprite.setPosition( gPoint.x, gPoint.y );

	V2d gn = mover->ground->Normal();
	dead = false;

	//----update the sprite
	//double angle = 0;



	//position = gPoint + gn * physBody.rh;//16.0;
	angle = atan2( gn.x, -gn.y );
		
	//sprite.setTexture( *ts_walk->texture );
	//sprite.setTextureRect( ts_walk->GetSubRect( frame / crawlAnimationFactor ) );
	//V2d pp = ground->GetPoint( edgeQuantity );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
	sprite.setRotation( angle / PI * 180 );
	sprite.setPosition( gPoint.x, gPoint.y );

	
	//----

	UpdateHitboxes();

	action = STAND;
	frame = 0;

	facingRight = false;

	UpdateSprite();
	//groundSpeed = 0;
}

void PoisonFrog::UpdateHitboxes()
{
	Edge *ground = mover->ground;
	if( ground != NULL )
	{
		V2d gn = ground->Normal();
		
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
		//hitBody.globalAngle = angle;
		//hurtBody.globalAngle = angle;
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	//hitBody.globalPosition = position + V2d( hitBody.offset.x * cos( hitBody.globalAngle ) + hitBody.offset.y * sin( hitBody.globalAngle ), hitBody.offset.x * -sin( hitBody.globalAngle ) + hitBody.offset.y * cos( hitBody.globalAngle ) );
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );

	hurtBody->GetCollisionBoxes(0)->front().globalPosition = position;
	hitBody->GetCollisionBoxes(0)->front().globalPosition = position;
}

void PoisonFrog::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case STAND:
			{
				action = JUMPSQUAT;
				frame = 0;
			}
			break;
		case JUMPSQUAT:
			{
				action = JUMP;
				frame = 0;
			}
			break;
		case JUMP:
			{
				frame = 1;
			}
			break;
		case LAND:
			{
				action = STAND;
				frame = 0;
			}
			break;
		case WALLCLING:
			{
				action = JUMP;
				frame = 1;

				if( facingRight )
				{
					mover->Jump( V2d( jumpStrength.x, -jumpStrength.y ) );
				}
				else
				{
					mover->Jump( V2d( -jumpStrength.x, -jumpStrength.y ) );
				}
				

				//frame = 0;
			}
			break;
		}
	}
}

void PoisonFrog::ProcessState()
{
	ActionEnded();

	Actor *player = owner->GetPlayer( 0 );

	V2d jumpVel;
	V2d gAlong;
	V2d gn;
	if( mover->ground != NULL )
	{
		gAlong = normalize( mover->ground->v1 - mover->ground->v0 );
		gn = mover->ground->Normal();
	}
	
	switch( action )
	{
	case STAND:
		{
			//cout << "frame: " << frame << endl;
			if( player->position.x < position.x )
			{
				facingRight = false;
			}
			else if( player->position.x > position.x )
			{
				facingRight = true;
			}
		}
		break;
	case JUMPSQUAT:
		{
		}
		break;
	case JUMP:
		{
			if( frame == 0 )
			{
				//jumpVel = 
				//cout << "jumping" << endl;
				if( facingRight )
				{
					//if( gn.x < 0 )
					if( cross( normalize( V2d( jumpStrength.x, -jumpStrength.y )), gAlong ) < 0 )
					{
						gAlong = (gAlong + V2d( 0, -1 )) / 2.0;
						mover->Jump( gAlong * jumpStrength.y );
					}
					else
					{
						mover->Jump( V2d( jumpStrength.x, -jumpStrength.y ) );
					}
					
				}
				else
				{
					if( cross( normalize( V2d( -jumpStrength.x, -jumpStrength.y )), gAlong ) < 0 )
					//if( gn.x > 0 )
					{
						gAlong = (-gAlong + V2d( 0, -1 )) / 2.0;
						mover->Jump( gAlong * jumpStrength.y );
					}
					else
					{
						mover->Jump( V2d( -jumpStrength.x, -jumpStrength.y ) );
					}
				}				
			}
			else
			{
				if (hasDoubleJump )
				{
					//cout << "vel: " << velocity.y << endl;
					V2d diff = player->position - position;
					if (mover->velocity.y > 3  && length(diff) < 300 && diff.y < 0)
					{
						hasDoubleJump = false;
						mover->velocity.y = -jumpStrength.y;
					}
				}


				
			}
		}
		break;
	case STEEPJUMP:
		{
			if( player->position.x < position.x )
			{
				facingRight = false;
			}
			else if( player->position.x > position.x )
			{
				facingRight = true;
			}

			if( facingRight )
			{
				mover->SetSpeed( 5 );
			}
			else
			{
				mover->SetSpeed( -5 );
			}
		}
		break;
	case LAND:
		{

		}
		break;
	case WALLCLING:
		{
		}
		break;
	}
}

void PoisonFrog::UpdateEnemyPhysics()
{
	//if (health == 0)
	//	return;

	if( mover->ground != NULL )
	{
	}
	else
	{
		double grav = gravity;
		if( action == WALLCLING )
		{
			grav = 0;//.1 * grav;
		}
		mover->velocity.y += grav / (numPhysSteps * slowMultiple);

		if( mover->velocity.y >= maxFallSpeed )
		{
			mover->velocity.y = maxFallSpeed;
		}
	}


	mover->Move( slowMultiple, numPhysSteps );

	position = mover->physBody.globalPosition;
}



void PoisonFrog::EnemyDraw(sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
}

void PoisonFrog::UpdateSprite()
{
	Edge *ground = mover->ground;
	double edgeQuantity = mover->edgeQuantity;
	V2d pp;
	
	if( ground != NULL )
	{
		pp = ground->GetPoint( edgeQuantity );
	}
	
	if( hasMonitor && !suppressMonitor )
	{
		keySprite->setTextureRect( ts_key->GetSubRect( owner->keyFrame / 2 ) );
		keySprite->setOrigin( keySprite->getLocalBounds().width / 2, 
			keySprite->getLocalBounds().height / 2 );
		keySprite->setPosition( position.x, position.y );
	}

	sprite.setTexture( *ts_test->texture );
		
	switch( action )
	{
	case STAND:
		{
			sprite.setTextureRect( ts_test->GetSubRect( 0 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case JUMPSQUAT:
		{
			sprite.setTextureRect( ts_test->GetSubRect( ( frame / animFactor[JUMPSQUAT] ) + 1 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case JUMP:
		{
			int window = 6;
			if( mover->velocity.y < -window )
			{
				sprite.setTextureRect( ts_test->GetSubRect( 3 ) );
			}
			else if( mover->velocity.y > window )
			{
				sprite.setTextureRect( ts_test->GetSubRect( 5 ) );
			}
			else
			{
				sprite.setTextureRect( ts_test->GetSubRect( 4 ) );
			}
			//sprite.setTextureRect( ts_test->GetSubRect( frame ) );
				
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			sprite.setPosition( position.x, position.y );
			sprite.setRotation( 0 );
		}
		break;
	case LAND:
		{
			sprite.setTextureRect( ts_test->GetSubRect( (frame / animFactor[LAND]) + 6 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
			sprite.setRotation( angle / PI * 180 );
			sprite.setPosition( pp.x, pp.y );
		}
		break;
	case WALLCLING:
		{
			sprite.setTextureRect( ts_test->GetSubRect( 8 ) );
			sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
			sprite.setRotation( 0 );//angle / PI * 180 );
			sprite.setPosition( position.x, position.y );
		}
		break;
	}

	if( !facingRight)
	{
		sf::IntRect r = sprite.getTextureRect();
		sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
	}

	//cout << "action: " << action << endl;
}

void PoisonFrog::HitTerrain( double &q )
{

}

bool PoisonFrog::StartRoll()
{
	return false;
}

void PoisonFrog::FinishedRoll()
{
	if( owner->IsSteepGround( mover->ground->Normal() ) == -1 )
	{
		action = JUMPSQUAT;
		mover->SetSpeed( 0 );
		frame = 0;
	}
}

void PoisonFrog::HitOther()
{
}

void PoisonFrog::ReachCliff()
{
	//if( owner->IsSteepGround( mover->ground->Normal() ) == -1 )
	{
		action = JUMPSQUAT;
		mover->SetSpeed( 0 );
		frame = 0;
	}
}

void PoisonFrog::HitOtherAerial( Edge *e )
{
	V2d norm = e->Normal();
	if( owner->IsWall( norm ) > 0 )
	{
		//if( action != WALLCLING )
		//{
			action = WALLCLING;
			frame = 0;
			//wallTouchCounter = 1;
		
			//if( norm.x > 0 ) //left wall, facing right
			//{
			//	mover->velocity = V2d( -.1, 0 );
			//}
			//else //right wall, facing left
			//{
			//	mover->velocity = V2d( .1, 0 );
			//}
			mover->velocity = V2d( 0, 0 );

		
			facingRight = !facingRight;
	}
}
	//}
	//if( 
void PoisonFrog::Land()
{
	//cout << "LANDING" << endl;
	action = LAND;
	frame = 0;
	hasDoubleJump = true;
	V2d gn = mover->ground->Normal();
	angle = atan2(gn.x, -gn.y);
}