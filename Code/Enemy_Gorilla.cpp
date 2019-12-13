#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Gorilla.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )


Gorilla::Gorilla( GameSession *owner, bool p_hasMonitor, Vector2i &pos, int p_level )
	:Enemy( owner, EnemyType::EN_GORILLA, p_hasMonitor, 1 ),approachAccelBez( 1,.01,.86,.32 ) 
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

	followFrames = 60;

	idealRadius = 300;
	
	recoveryLoops = 1;
	wallWidth = 400;
	wallHitboxWidth = wallWidth;
	wallHitboxHeight = 50; //wallHeight;

	wallHitbox.isCircle = false;
	wallHitbox.rw = wallHitboxWidth / 2;
	wallHitbox.rh = wallHitboxHeight / 2;

	actionLength[WAKEUP] = 60;
	actionLength[ALIGN] = 60;
	actionLength[FOLLOW] = followFrames;
	actionLength[ATTACK] = 4;//60;
	actionLength[RECOVER] = 2;//60;

	actionLength[WAKEUP] = 1;
	animFactor[ALIGN] = 1;
	animFactor[FOLLOW] = 1;
	animFactor[ATTACK] = 15;
	animFactor[RECOVER] = 60;

	action = WAKEUP;

	latchedOn = false;

	awakeCap = 60;
	//offsetPlayer 
	receivedHit = NULL;
	position.x = pos.x;
	position.y = pos.y;

	origPosition = position;

	approachFrames = 180 * 3;
	totalFrame = 0;

	spawnRect = sf::Rect<double>( pos.x - 64, pos.y - 64, 64 * 2, 64 * 2 );

	basePos = position;

	animationFactor = 5;

	//ts = owner->GetTileset( "Gorilla.png", 80, 80 );
	ts = owner->GetTileset( "Enemies/gorilla_320x256.png", 320, 256 );

	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setScale(scale, scale);
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	
	//position.x = 0;
	//position.y = 0;

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	SetupBodies(1, 1);
	AddBasicHurtCircle(48);
	AddBasicHitCircle(48);
	hitBody->hitboxInfo = hitboxInfo;

	ts_wall = owner->GetTileset( "Enemies/gorillawall_400x50.png", 400, 50 );
	wallSprite.setTexture( *ts_wall->texture );
	wallSprite.setTextureRect( ts_wall->GetSubRect( 0 ) );
	wallSprite.setScale(scale, scale);
	wallSprite.setOrigin( wallSprite.getLocalBounds().width / 2, wallSprite.getLocalBounds().height / 2 );

	facingRight = true;

	origFacingRight = facingRight;

	createWallFrame = 20;
}

void Gorilla::ResetEnemy()
{
	action = WAKEUP;
	facingRight = origFacingRight;

	awakeFrames = 0;
	latchStartAngle = 0;
	latchedOn = false;
	totalFrame = 0;
	
	dead = false;
	frame = 0;
	basePos = origPosition;
	position = basePos;
	
	receivedHit = NULL;
	recoveryCounter = 0;

	UpdateHitboxes();
	UpdateSprite();
	
}

void Gorilla::ActionEnded()
{
	if( frame == actionLength[action] * animFactor[action] )
	{
		switch( action )
		{
		case WAKEUP:
			frame = 0;
			break;
		case ALIGN:
			frame = 0;
			break;
		case FOLLOW:
			action = ATTACK;
			frame = 0;
			break;
		case ATTACK:
			recoveryCounter = 0;
			action = RECOVER;
			frame = 0;
			break;
		case RECOVER:
			++recoveryCounter;
			//if( recoveryCounter == recoveryLoops )
			{
				latchedOn = true;
				offsetPlayer = basePos - owner->GetPlayer( 0 )->position;
				origOffset = offsetPlayer;
				V2d offsetDir = normalize( offsetPlayer );

				basePos = owner->GetPlayer( 0 )->position;


				V2d playerPos = owner->GetPlayer( 0 )->position;
				action = ALIGN;
				double currRadius = length( offsetPlayer );
				alignMoveFrames = 60 * 5 * NUM_STEPS;
				frame = 0;
				alignFrames = 0;
				recoveryCounter = 0;

				if( playerPos.x < position.x )
				{
					facingRight = false;
				}
				else
				{
					facingRight = true;
				}
			}
			break;
		}
	}
}

void Gorilla::ProcessState()
{
	ActionEnded();

	V2d playerPos = owner->GetPlayer( 0 )->position;

	switch( action )
	{
	case WAKEUP:
		{
			if( WithinDistance( position, playerPos, 400 ) )
			{
				++awakeFrames;
				if( awakeFrames == awakeCap )
				{
					//awake = true;
					action = ALIGN;
					alignFrames = 0;
					frame = 0;
					if( playerPos.x < position.x )
					{
						facingRight = false;
					}
					else
					{
						facingRight = true;
					}

					latchedOn = true;
					offsetPlayer = basePos - owner->GetPlayer( 0 )->position;
					origOffset = offsetPlayer;
					V2d offsetDir = normalize(offsetPlayer);

					basePos = owner->GetPlayer( 0 )->position;

					double currRadius = length( offsetPlayer );
					alignMoveFrames = 60 * 5 * NUM_STEPS;
					
				}
			}
			else
			{
				awakeFrames--;
				if( awakeFrames < 0 )
					awakeFrames = 0;
			}
		}
		break;
	case ALIGN:
		{
			if( alignFrames >= alignMoveFrames )
			{
				action = FOLLOW;
				frame = 0;
			}
		}
		break;
	case FOLLOW:
		{
			//cout << "follow" << endl;	
		}
		break;
	case ATTACK:

		//cout << "attack" << endl;
		if( frame == createWallFrame )
		{
			V2d test = position - playerPos;
	
			V2d playerDir = -normalize( origOffset );
			wallHitbox.globalPosition = position + playerDir * 100.0;
			wallHitbox.globalAngle = atan2( playerDir.x, -playerDir.y );

			wallSprite.setPosition( wallHitbox.globalPosition.x, 
				wallHitbox.globalPosition.y );
			wallSprite.setRotation( wallHitbox.globalAngle / PI * 180.0 );

			latchedOn = false;
			basePos = position;
		}
		break;
	case RECOVER:
		//cout << "recover" << endl;
		break;
	}
}

void Gorilla::UpdateEnemyPhysics()
{
	if( latchedOn )
	{
		basePos = owner->GetPlayer( 0 )->position;// + offsetPlayer;
	}
	else
	{
		
		
	}

	V2d offsetDir = normalize( offsetPlayer );

	switch( action )
	{
	case WAKEUP:
		//frame = 0;
		break;
	case ALIGN:
		{
			V2d idealOffset = offsetDir * idealRadius;
			CubicBezier b( 0, 0, 1, 1 );
			double f = b.GetValue( (alignFrames + owner->substep) / (double)alignMoveFrames );
			offsetPlayer = origOffset * (1.0 - f) + idealOffset * f;
			alignFrames += 5 / slowMultiple;
		}
		//frame = 0;
		break;
	case FOLLOW:
		//cout << "follow: " << frame << endl;
		//frame = 0;
		break;
	case ATTACK:
		//cout << "ATTACK: " << frame << endl;
		//action = RECOVER;
		//frame = 0;
		break;
	case RECOVER:
		//cout << "recovery: " << frame << endl;
		//action = ALIGN;
		//frame = 0;
		break;
	}

	if( latchedOn )
	{
		position = basePos + offsetPlayer;
	}
}


void Gorilla::UpdateSprite()
{
	V2d diff = owner->GetPlayer(0)->position - position;
	double lenDiff = length(diff);
	IntRect ir;
	switch (action)
	{
	case WAKEUP:
		ir = ts->GetSubRect(0);
		break;
	case ALIGN:
		ir = ts->GetSubRect(0);
		break;
	case FOLLOW:
		ir = ts->GetSubRect(1);
		break;
	case ATTACK:
		ir = ts->GetSubRect(frame / animFactor[ATTACK] + 2);
		break;
	case RECOVER:
		ir = ts->GetSubRect(frame / animFactor[RECOVER] + 6);
		break;
	}

	if (!facingRight)
	{
		ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
	}

	sprite.setTextureRect(ir);

	sprite.setPosition(position.x, position.y);
}

void Gorilla::EnemyDraw( sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);

	if ((action == ATTACK && frame > createWallFrame) || action == RECOVER)
	{
		target->draw(wallSprite);
	}


}


void Gorilla::UpdateHitboxes()
{
	CollisionBox &hurtBox = hurtBody->GetCollisionBoxes(0)->front();
	CollisionBox &hitBox = hitBody->GetCollisionBoxes(0)->front();
	hurtBox.globalPosition = position;
	hurtBox.globalAngle = 0;
	hitBox.globalPosition = position;
	hitBox.globalAngle = 0;

	if( owner->GetPlayer( 0 )->ground != NULL )
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->groundSpeed * ( owner->GetPlayer( 0 )->ground->v1 - owner->GetPlayer( 0 )->ground->v0 ) );
	}
	else
	{
		hitboxInfo->kbDir = normalize( -owner->GetPlayer( 0 )->velocity );
	}
}