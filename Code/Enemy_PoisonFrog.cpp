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

void PoisonFrog::SetLevel(int lev)
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

PoisonFrog::PoisonFrog( ActorParams *ap )
	:Enemy( EnemyType::EN_POISONFROG, ap )
{
	SetLevel(ap->GetLevel());

	SetNumActions(Count);
	SetEditorActions(STAND, STAND, 0);
	
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

	gravityFactor = 30;
	jumpFramesWait = 60;
	jumpStrength = V2d(5, 12);

	maxFallSpeed = 25;

	invincibleFrames = 0;
	double width = 80;
	double height = 80;
	ts_test = sess->GetSizedTileset( "Enemies/frog_80x80.png");

	xSpeed = 8;

	sprite.setTexture(*ts_test->texture);

	CreateGroundMover(startPosInfo, 30, true, this);
	groundMover->SetSpeed( 0 );
	
	gravity = gravityFactor / 64.0;

	sprite.setTexture( *ts_test->texture );
	sprite.setScale(scale, scale);

	BasicCircleHurtBodySetup(30);
	BasicCircleHitBodySetup(30);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 0;

	hitBody.hitboxInfo = hitboxInfo;

	cutObject->Setup(ts_test, 10, 9, scale);

	//cutObject->SetTileset(ts_test);
	//cutObject->SetSubRectFront(10);
	//cutObject->SetSubRectBack(9);
	//cutObject->SetScale(scale);

	ResetEnemy();
}

void PoisonFrog::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}

void PoisonFrog::ResetEnemy()
{
	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	invincibleFrames = 0;
	hasDoubleJump = true;

	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	UpdateHitboxes();

	action = STAND;
	frame = 0;

	facingRight = false;

	UpdateSprite();
}

void PoisonFrog::UpdateHitboxes()
{
	BasicUpdateHitboxes();

	//this isnt figured out yet. probably automate this at some point to make it simpler
	//Edge *ground = groundMover->ground;
	//if( ground != NULL )
	//{
	//	V2d gn = ground->Normal();
	//	
	//	V2d knockbackDir( 1, -1 );
	//	knockbackDir = normalize( knockbackDir );
	//	if(groundMover->groundSpeed > 0 )
	//	{
	//		hitboxInfo->kbDir = knockbackDir;
	//		hitboxInfo->knockback = 15;
	//	}
	//	else
	//	{
	//		hitboxInfo->kbDir = V2d( -knockbackDir.x, knockbackDir.y );
	//		hitboxInfo->knockback = 15;
	//	}
	//	//hitBody.globalAngle = angle;
	//	//hurtBody.globalAngle = angle;
	//}
	//else
	//{
	//	//hitBody.globalAngle = 0;
	//	//hurtBody.globalAngle = 0;
	//}
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
					groundMover->Jump( V2d( jumpStrength.x, -jumpStrength.y ) );
				}
				else
				{
					groundMover->Jump( V2d( -jumpStrength.x, -jumpStrength.y ) );
				}
			}
			break;
		}
	}
}

void PoisonFrog::ProcessState()
{
	ActionEnded();

	V2d playerPos = sess->GetPlayerPos(0);

	V2d jumpVel;
	V2d gAlong;
	V2d gn;
	if(groundMover->ground != NULL )
	{
		gAlong = normalize(groundMover->ground->v1 - groundMover->ground->v0 );
		gn = groundMover->ground->Normal();
	}
	
	switch( action )
	{
	case STAND:
		{
			//cout << "frame: " << frame << endl;
			if(playerPos.x < GetPosition().x )
			{
				facingRight = false;
			}
			else if(playerPos.x > GetPosition().x )
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
						groundMover->Jump( gAlong * jumpStrength.y );
					}
					else
					{
						groundMover->Jump( V2d( jumpStrength.x, -jumpStrength.y ) );
					}
					
				}
				else
				{
					if( cross( normalize( V2d( -jumpStrength.x, -jumpStrength.y )), gAlong ) < 0 )
					//if( gn.x > 0 )
					{
						gAlong = (-gAlong + V2d( 0, -1 )) / 2.0;
						groundMover->Jump( gAlong * jumpStrength.y );
					}
					else
					{
						groundMover->Jump( V2d( -jumpStrength.x, -jumpStrength.y ) );
					}
				}				
			}
			else
			{
				if (hasDoubleJump )
				{
					//cout << "vel: " << velocity.y << endl;
					V2d diff = playerPos - GetPosition();
					if (groundMover->velocity.y > 3  && length(diff) < 300 && diff.y < 0)
					{
						hasDoubleJump = false;
						groundMover->velocity.y = -jumpStrength.y;
					}
				}


				
			}
		}
		break;
	case STEEPJUMP:
		{
			if(playerPos.x < GetPosition().x )
			{
				facingRight = false;
			}
			else if(playerPos.x > GetPosition().x )
			{
				facingRight = true;
			}

			if( facingRight )
			{
				groundMover->SetSpeed( 5 );
			}
			else
			{
				groundMover->SetSpeed( -5 );
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
	if (numHealth > 0)
	{
		if (groundMover->ground == NULL)
		{
			double grav = gravity;
			if (action == WALLCLING)
			{
				grav = 0;//.1 * grav;
			}
			groundMover->velocity.y += grav / (numPhysSteps * slowMultiple);

			if (groundMover->velocity.y >= maxFallSpeed)
			{
				groundMover->velocity.y = maxFallSpeed;
			}
		}

		Enemy::UpdateEnemyPhysics();
	}
}

void PoisonFrog::EnemyDraw(sf::RenderTarget *target )
{
	DrawSprite(target, sprite, auraSprite);
}

void PoisonFrog::UpdateSprite()
{	
	int currTile = 0;
	switch( action )
	{
	case STAND:
		{
			currTile = 0;
		}
		break;
	case JUMPSQUAT:
		{
		currTile = frame / animFactor[JUMPSQUAT] + 1;
		}
		break;
	case JUMP:
		{
			int window = 6;
			if( groundMover->velocity.y < -window )
			{
				currTile = 3;
			}
			else if(groundMover->velocity.y > window )
			{
				currTile = 5;
			}
			else
			{
				currTile = 4;
			}
		}
		break;
	case LAND:
		{
		currTile = frame / animFactor[LAND] + 6;
		}
		break;
	case WALLCLING:
		{
		currTile = 8;
			
		}
		break;
	}

	ts_test->SetSubRect(sprite, currTile, !facingRight);
	if (action == STAND || action == JUMPSQUAT || action == LAND)
	{
		sprite.setRotation(groundMover->GetAngleDegrees());
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height);
		sprite.setPosition(groundMover->GetGroundPointF());
	}
	else
	{
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
		sprite.setRotation(0);
		sprite.setPosition(GetPositionF());
	}
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
	if( groundMover->ground->IsSteepGround() )
	{
		action = JUMPSQUAT;
		groundMover->SetSpeed( 0 );
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
		groundMover->SetSpeed( 0 );
		frame = 0;
	}
}

void PoisonFrog::HitOtherAerial( Edge *e )
{
	V2d norm = e->Normal();
	if( e->IsWall() )
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
			groundMover->velocity = V2d( 0, 0 );

		
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
	V2d gn = groundMover->ground->Normal();
	angle = atan2(gn.x, -gn.y);
}