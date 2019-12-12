#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Ghost.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_RED Color( 0xff, 0x22, 0 )


Ghost::Ghost( GameSession *owner, bool p_hasMonitor, Vector2i &pos, int p_level )
	:Enemy( owner, EnemyType::EN_GHOST, p_hasMonitor, 1 ), approachAccelBez( 1,.01,.86,.32 ) 
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

	detectionRadius = 600;

	actionLength[WAKEUP] = 60;
	actionLength[APPROACH] = 2;
	actionLength[BITE] = 4;
	actionLength[EXPLODE] = 9;

	animFactor[WAKEUP] = 1;
	animFactor[APPROACH] = 20;
	animFactor[BITE] = 5;
	animFactor[EXPLODE] = 7;

	action = WAKEUP;


	position.x = pos.x;
	position.y = pos.y;

	origPosition = position;

	approachFrames = 180 * 3;
	totalFrame = 0;

	V2d playerPos = owner->GetPlayer(0)->position;
	V2d dirFromPlayer = normalize( playerPos  - position );
	double fromPlayerAngle =  atan2( dirFromPlayer.y, dirFromPlayer.x ) + PI;
	testSeq.AddRadialMovement( V2d( 0, 0 ), 1, 0, 2 * PI * 3, 
		true, V2d( 1, 1 ), 0, CubicBezier( 0, 0, 1, 1), approachFrames );
	
	testSeq.InitMovementDebug();

	spawnRect = sf::Rect<double>( pos.x - 64, pos.y - 64, 64 * 2, 64 * 2 );

	basePos = position;
	
	speed = 1;

	//speed = 2;
	frame = 0;

	//ts = owner->GetTileset( "Ghost.png", 80, 80 );
	ts = owner->GetTileset( "Enemies/plasmid_192x192.png", 192, 192 );
	sprite.setTexture( *ts->texture );
	sprite.setTextureRect( ts->GetSubRect( frame ) );
	sprite.setOrigin( sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2 );
	sprite.setPosition( pos.x, pos.y );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	SetupBodies(1, 1);
	AddBasicHurtCircle(32);
	AddBasicHitCircle(32);
	hitBody->hitboxInfo = hitboxInfo;

	awakeFrames = 0;
	awakeCap = 60;
	
	latchStartAngle = 0;
	dead = false;

	facingRight = (playerPos.x - position.x) >= 0;

	UpdateHitboxes();

	origFacingRight = facingRight;

	ResetEnemy();
}

void Ghost::ResetEnemy()
{
	action = WAKEUP;
	facingRight = origFacingRight;
	awakeFrames = 0;
	latchStartAngle = 0;
	latchedOn = false;
	totalFrame = 0;
	testSeq.Reset();

	dead = false;
	
	frame = 0;
	basePos = origPosition;
	position = basePos;

	sprite.setColor(Color( 255, 255, 255, 100 ));
	
	receivedHit = NULL;

	UpdateHitboxes();
	UpdateSprite();
	
}

void Ghost::Bite()
{
	offsetPlayer = V2d(0, 0);
	action = BITE;
	frame = 0;
	sprite.setColor(Color::White);
	SetHurtboxes(hurtBody, 0);
}

void Ghost::ProcessState()
{
	
	if( frame == actionLength[action] * animFactor[action] )
	{
		frame = 0;

		if (action == BITE)
		{
			action = EXPLODE;
			SetHitboxes(hitBody, 0);
			SetHurtboxes(NULL, 0);
		}
		else if( action == EXPLODE )
		{
			numHealth = 0;
			dead = true;
		}
		
	}

	if (action == EXPLODE && frame == 1 && slowCounter == 1)
	{
		SetHitboxes(NULL, 0);
	}

	if( action == APPROACH && offsetPlayer.x == 0 && offsetPlayer.y == 0 )
	{
		assert(0); //should this even bit hit?
		Bite();
	}

	/*if (action == APPROACH)
	{
		double d = length(offsetPlayer);
		cout << "offsetDist: " << d << endl;
	}*/

	V2d playerPos = owner->GetPlayer(0)->position;
	if (action == WAKEUP)
	{
		if( WithinDistance( playerPos, position, 600 ))
		{
			awakeFrames++;

			if (awakeFrames == awakeCap)
			{
				action = APPROACH;
				frame = 0;
				if (playerPos.x < position.x)
				{
					facingRight = false;
				}
				else
				{
					facingRight = true;
				}

				latchedOn = true;
				offsetPlayer = basePos - owner->GetPlayer(0)->position;//owner->GetPlayer( 0 )->position - basePos;
				origOffset = offsetPlayer;//length( offsetPlayer );
				V2d offsetDir = normalize(offsetPlayer);
				basePos = owner->GetPlayer(0)->position;
			}
		}
		else
		{
			awakeFrames--;
			if (awakeFrames < 0)
				awakeFrames = 0;
		}
	}
}

void Ghost::UpdateEnemyPhysics()
{
	if (latchedOn)
	{
		basePos = owner->GetPlayer(0)->position;

		
		if (action == APPROACH && latchedOn)
		{
			offsetPlayer += -normalize(offsetPlayer) * 1.0 / numPhysSteps;

			if (length(offsetPlayer) < 1.0)
			{
				Bite();
			}
		}

		position = basePos + offsetPlayer;
	}
}
void Ghost::UpdateSprite()
{
	if (latchedOn)
	{
		V2d playerPos = owner->GetPlayer(0)->position;
		basePos = playerPos;
		position = basePos + offsetPlayer;
	}
	
	double lenDiff = length(offsetPlayer);//owner->GetPlayer(0)->position - position;
	//length(diff);
	IntRect ir;
	switch (action)
	{
	case WAKEUP:
		ir = ts->GetSubRect(0);
		break;
	case APPROACH:
		if (lenDiff < 100)
		{
			ir = ts->GetSubRect(5);
		}
		else if (lenDiff < 200)
		{
			ir = ts->GetSubRect(4);
		}
		else if (lenDiff < 300)
		{
			ir = ts->GetSubRect(3);
		}
		else if (lenDiff < 400)
		{
			ir = ts->GetSubRect(2);
		}
		else if (lenDiff < 500)
		{
			ir = ts->GetSubRect(1);
		}
		else if (lenDiff < 600)
		{
			ir = ts->GetSubRect(0);
		}
		break;
	case BITE:
		ir = ts->GetSubRect((frame / animFactor[BITE]) + 6);
		break;
	case EXPLODE:
		ir = ts->GetSubRect(frame / animFactor[EXPLODE] + 10);
		break;
	}

	if (!facingRight)
	{
		ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
	}

	sprite.setTextureRect(ir);


	sprite.setPosition(position.x, position.y);
}

void Ghost::EnemyDraw( sf::RenderTarget *target )
{
	DrawSpriteIfExists(target, sprite);
}

void Ghost::DrawMinimap( sf::RenderTarget *target )
{
}

void Ghost::UpdateHitboxes()
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