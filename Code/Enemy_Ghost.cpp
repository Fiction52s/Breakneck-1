#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Ghost.h"

using namespace std;
using namespace sf;

Ghost::Ghost( ActorParams *ap )
	:Enemy( EnemyType::EN_GHOST, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(BITE, APPROACH, 0);

	SetLevel(ap->GetLevel());

	approachSpeed = 5.0;

	detectionRadius = 600;
	awakeCap = 30;

	actionLength[WAKEUP] = 30;
	actionLength[APPROACH] = 2;
	actionLength[BITE] = 7;
	actionLength[EXPLODE] = 12;
	actionLength[RETURN] = 30;

	animFactor[WAKEUP] = 1;
	animFactor[APPROACH] = 20;
	animFactor[BITE] = 5;
	animFactor[EXPLODE] = 3;
	animFactor[RETURN] = 1;

	ts = GetSizedTileset("Enemies/W5/plasmid_192x192.png");
	sprite.setTexture( *ts->texture );
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 30;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::RED;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(40);

	hitBody.hitboxInfo = hitboxInfo;

	cutObject->SetTileset(ts);
	cutObject->SetScale(scale);

	ResetEnemy();
}

void Ghost::ResetEnemy()
{
	action = WAKEUP;
	data.awakeFrames = 0;
	data.latchStartAngle = 0;
	data.latchedOn = false;
	data.totalFrame = 0;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	
	facingRight = (sess->GetPlayerPos(0).x - GetPosition().x) >= 0;

	frame = 0;
	data.basePos = startPosInfo.position;

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
	//data.offsetPlayer = V2d(0, 0);
	action = BITE;
	frame = 0;
	sprite.setColor(Color::White);
	
}

void Ghost::ProcessState()
{
	
	if( frame == actionLength[action] * animFactor[action] )
	{
		frame = 0;

		if (action == BITE)
		{
			action = EXPLODE;
			HitboxesOff();
			//DefaultHitboxesOn();
			HurtboxesOff();
		}
		else if( action == EXPLODE )
		{
			action = RETURN;
			data.offsetPlayer = data.origOffset;
			sprite.setColor(Color(255, 255, 255, 100));

			//HitboxesOn();
			//HitboxesOff();
			//HurtboxesOff();
			//numHealth = 0;
			//dead = true;
		}
		else if (action == RETURN)
		{
			action = APPROACH;
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
		}
		
	}

	if (action == EXPLODE && frame == 1 && slowCounter == 1)
	{
		SetHitboxes(NULL, 0);
	}

	//if( action == APPROACH && data.offsetPlayer.x == 0 && data.offsetPlayer.y == 0 )
	//{
	//	assert(0); //should this even bit hit?
	//	Bite();
	//}

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
			data.awakeFrames++;

			if (data.awakeFrames == awakeCap)
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

				data.latchedOn = true;
				data.offsetPlayer = data.basePos - playerPos;//owner->GetPlayer( 0 )->position - basePos;
				data.origOffset = data.offsetPlayer;//length( offsetPlayer );
				V2d offsetDir = normalize(data.offsetPlayer);
				data.basePos = playerPos;
			}
		}
		else
		{
			data.awakeFrames--;
			if (data.awakeFrames < 0)
				data.awakeFrames = 0;
		}
	}
}

void Ghost::UpdateEnemyPhysics()
{
	if (data.latchedOn)
	{
		data.basePos = sess->GetPlayerPos(0);


		if (( action == APPROACH || action == BITE ) && data.latchedOn)
		{
			data.offsetPlayer += -normalize(data.offsetPlayer) * 1.0 / numPhysSteps * approachSpeed;

			if ( action == APPROACH && length(data.offsetPlayer) < 80 )//1.0)
			{
				Bite();
			}
		}

		currPosInfo.position = data.basePos + data.offsetPlayer;
	}
}
void Ghost::UpdateSprite()
{
	if (data.latchedOn)
	{
		V2d playerPos = sess->GetPlayerPos(0);
		data.basePos = playerPos;
		currPosInfo.position = data.basePos + data.offsetPlayer;
	}
	
	double lenDiff = length(data.offsetPlayer);//owner->GetPlayer(0)->position - position;
	//length(diff);
	IntRect ir;
	switch (action)
	{
	case WAKEUP:
		ir = ts->GetSubRect(0);
		break;
	case APPROACH:
		if (lenDiff < 300)
		{
			ir = ts->GetSubRect(4);
		}
		else if (lenDiff < 400)
		{
			ir = ts->GetSubRect(3);
		}
		else if (lenDiff < 500)
		{
			ir = ts->GetSubRect(2);
		}
		else
		{
			ir = ts->GetSubRect(1);
		}
		break;
	case BITE:
		ir = ts->GetSubRect((frame / animFactor[BITE]) + 5);
		break;
	case EXPLODE:
		ir = ts->GetSubRect(frame / animFactor[EXPLODE] + 12);
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

int Ghost::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Ghost::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Ghost::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}

void Ghost::HandleNoHealth()
{
	if (action == WAKEUP)
	{
		cutObject->SetSubRectFront(24);
		cutObject->SetSubRectBack(25);
	}
	else if (action == APPROACH)
	{
		double lenDiff = length(data.offsetPlayer);
		if (lenDiff > 300)
		{
			cutObject->SetSubRectFront(26);
			cutObject->SetSubRectBack(27);
		}
		else
		{
			cutObject->SetSubRectFront(28);
			cutObject->SetSubRectBack(29);
		}
	}
	else
	{
		cutObject->SetSubRectFront(28);
		cutObject->SetSubRectBack(29);
	}
}