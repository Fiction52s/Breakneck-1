#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Ghost.h"

using namespace std;
using namespace sf;

Ghost::Ghost( ActorParams *ap )
	:Enemy( EnemyType::EN_GHOST, ap), approachAccelBez( 1,.01,.86,.32 ) 
{
	SetNumActions(A_Count);
	SetEditorActions(BITE, APPROACH, 0);

	SetLevel(ap->GetLevel());

	detectionRadius = 600;
	approachFrames = 180 * 3;
	speed = 1;
	awakeCap = 60;

	actionLength[WAKEUP] = 60;
	actionLength[APPROACH] = 2;
	actionLength[BITE] = 4;
	actionLength[EXPLODE] = 5;
	actionLength[RETURN] = 30;

	animFactor[WAKEUP] = 1;
	animFactor[APPROACH] = 20;
	animFactor[BITE] = 5;
	animFactor[EXPLODE] = 7;
	animFactor[RETURN] = 1;

	ts = sess->GetSizedTileset("Enemies/W5/plasmid_192x192.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Ghost::ResetEnemy()
{
	action = WAKEUP;
	awakeFrames = 0;
	latchStartAngle = 0;
	latchedOn = false;
	totalFrame = 0;
	
	facingRight = (sess->GetPlayerPos(0).x - GetPosition().x) >= 0;

	frame = 0;
	basePos = startPosInfo.position;

	sprite.setColor(Color( 255, 255, 255, 100 ));

	UpdateHitboxes();
	UpdateSprite();
}

void Ghost::SetLevel(int lev)
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

void Ghost::Bite()
{
	offsetPlayer = V2d(0, 0);
	action = BITE;
	frame = 0;
	sprite.setColor(Color::White);
	DefaultHurtboxesOn();
}

void Ghost::ProcessState()
{
	
	if( frame == actionLength[action] * animFactor[action] )
	{
		frame = 0;

		if (action == BITE)
		{
			action = EXPLODE;
			DefaultHitboxesOn();
			HurtboxesOff();
		}
		else if( action == EXPLODE )
		{
			action = RETURN;
			offsetPlayer = origOffset;
			sprite.setColor(Color(255, 255, 255, 100));

			HitboxesOff();
			HurtboxesOff();
			//numHealth = 0;
			//dead = true;
		}
		else if (action == RETURN)
		{
			action = APPROACH;
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

	V2d playerPos = sess->GetPlayerPos(0);
	if (action == WAKEUP)
	{
		if( WithinDistance( playerPos, GetPosition(), 1000 ))
		{
			awakeFrames++;

			if (awakeFrames == awakeCap)
			{
				action = APPROACH;
				frame = 0;
				if (playerPos.x < GetPosition().x)
				{
					facingRight = false;
				}
				else
				{
					facingRight = true;
				}

				latchedOn = true;
				offsetPlayer = basePos - playerPos;//owner->GetPlayer( 0 )->position - basePos;
				origOffset = offsetPlayer;//length( offsetPlayer );
				V2d offsetDir = normalize(offsetPlayer);
				basePos = playerPos;
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
		basePos = sess->GetPlayerPos(0);

		
		if (action == APPROACH && latchedOn)
		{
			offsetPlayer += -normalize(offsetPlayer) * 1.0 / numPhysSteps;

			if (length(offsetPlayer) < 1.0)
			{
				Bite();
			}
		}

		currPosInfo.position = basePos + offsetPlayer;
	}
}
void Ghost::UpdateSprite()
{
	if (latchedOn)
	{
		V2d playerPos = sess->GetPlayerPos(0);
		basePos = playerPos;
		currPosInfo.position = basePos + offsetPlayer;
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
		else
		{
			ir = ts->GetSubRect(1);
		}
		break;
	case BITE:
		ir = ts->GetSubRect((frame / animFactor[BITE]) + 6);
		break;
	case EXPLODE:
		ir = ts->GetSubRect(frame / animFactor[EXPLODE] + 10);
		break;
	case RETURN:
		ir = ts->GetSubRect(0);
		break;
	}

	if (!facingRight)
	{
		ir = sf::IntRect(ir.left + ir.width, ir.top, -ir.width, ir.height);
	}

	sprite.setTextureRect(ir);
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Ghost::EnemyDraw( sf::RenderTarget *target )
{
	DrawSprite(target, sprite);
}

void Ghost::DrawMinimap( sf::RenderTarget *target )
{
}