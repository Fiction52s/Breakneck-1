#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Dragon.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Dragon::Dragon(ActorParams *ap)
	:Enemy(EnemyType::EN_DRAGON, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 2;
	actionLength[FLY] = 7;
	actionLength[RUSH] = 8; //2

	recoverDuration = 60;

	animFactor[NEUTRAL] = 5;
	animFactor[FLY] = 3;
	animFactor[RUSH] = 3;//12;//8;

	rushSpeed = 25;//30;


	accel.x = 1.0;//.6;
	accel.y = 2.0;//.7;//.3;

	maxSpeed.x = 17;//20;
	maxSpeed.y = 17;//10;

	ts = GetSizedTileset("Enemies/W6/rusher_160x160.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(13);
	cutObject->SetSubRectBack(12);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHitBodySetup(50);
	BasicCircleHurtBodySetup(70);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Dragon::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Dragon::SetLevel(int lev)
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

void Dragon::FlyMovement()
{
	if (false)//(PlayerDist() < 100)
	{
		data.velocity = V2d(0, 0);
	}
	else
	{
		V2d playerDir = PlayerDir(V2d(), testOffsetDir * 100.0);
		data.velocity.x += playerDir.x * accel.x;
		data.velocity.y += playerDir.y * accel.y;

		if (data.velocity.x > maxSpeed.x)
		{
			data.velocity.x = maxSpeed.x;
		}
		else if (data.velocity.x < -maxSpeed.x)
		{
			data.velocity.x = -maxSpeed.x;
		}

		if (data.velocity.y > maxSpeed.y)
		{
			data.velocity.y = maxSpeed.y;
		}
		else if (data.velocity.y < -maxSpeed.y)
		{
			data.velocity.y = -maxSpeed.y;
		}

		if (playerDir.x >= 0)
		{
			facingRight = true;
		}
		else
		{
			facingRight = false;
		}
	}
}

void Dragon::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	data.numPastPositions = 0;

	for (int i = 0; i < MAX_PAST_POSITIONS; ++i)
	{
		data.pastPositions[i] = V2d();
	}

	for (int i = 0; i < NUM_SEGMENTS; ++i)
	{
		ClearRect(segmentQuads + i * 4);
		SetRectSubRect(segmentQuads + i * 4, ts->GetSubRect(0));
	}


	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	data.recoverFrame = 0;

	data.velocity = V2d();

	UpdateHitboxes();

	UpdateSprite();
}

void Dragon::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case FLY:
			break;
		case RUSH:
			action = FLY;
			data.recoverFrame = 0;
			break;
		}
	}
}

void Dragon::FrameIncrement()
{
	if (action == FLY)
	{
		++data.recoverFrame;
	}
}

void Dragon::AddPastPosition(V2d pos)
{
	for (int i = data.numPastPositions - 1; i > 0; --i)
	{
		data.pastPositions[i] = data.pastPositions[i - 1];
	}

	data.pastPositions[0] = pos;

	if (data.numPastPositions < MAX_PAST_POSITIONS)
	{
		++data.numPastPositions;
	}
}

void Dragon::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();
	

	switch (action)
	{
	case NEUTRAL:
	{
		double yDiff = PlayerDiff().y;
		if (abs(PlayerDiff().x) < 700 && ((yDiff >=0 && yDiff < 400) || ( yDiff <= 0 && yDiff > -700)) )//dist < 400 )//DEFAULT_DETECT_RADIUS)
		{
			action = FLY;
			data.recoverFrame = 0;
			frame = 0;
			testOffsetDir = -dir;
		}
		break;
	}	
	case FLY:
		if (dist > DEFAULT_IGNORE_RADIUS)
		{
			action = NEUTRAL;
			frame = 0;
		}
		else if (data.recoverFrame == recoverDuration)
		{
			action = RUSH;
			frame = 0;
			data.velocity = dir * rushSpeed;
		}
		break;
	case RUSH:



		break;
	}

	switch (action)
	{
	case NEUTRAL:
		data.velocity = V2d(0, 0);
		break;
	case FLY:
		FlyMovement();
		break;
	case RUSH:
		break;
	}

	AddPastPosition(GetPosition());
}

void Dragon::UpdateEnemyPhysics()
{
	if (action == FLY || action == RUSH)
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
}

void Dragon::UpdateSprite()
{
	int tile = 0;

	switch (action)
	{
	case NEUTRAL:
		tile = 0;//frame / animFactor[NEUTRAL];
		sprite.setRotation(0);
		break;
	case FLY:
	{
		tile = 2;//frame / animFactor[FLY];
		//sprite.setRotation(0);

		double ang = GetVectorAngleCW(data.velocity);
		if (facingRight)
		{
			sprite.setRotation(ang / PI * 180.0);
		}
		else
		{
			sprite.setRotation(ang / PI * 180.0 + 180);
		}
		break;
	}
	case RUSH:
	{
		tile = frame / animFactor[RUSH] + 1;

		double ang = GetVectorAngleCW(data.velocity);
		if (facingRight)
		{
			sprite.setRotation(ang / PI * 180.0);
		}
		else
		{
			sprite.setRotation(ang / PI * 180.0 + 180);
		}
		break;
	}
	}
	ts->SetSubRect(sprite, tile, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	/*for (int i = 0; i < data.numPastPositions; ++i)
	{
		SetRectCenter(segmentQuads + i * 4, ts->tileWidth, ts->tileHeight, Vector2f(data.pastPositions[i]));
	}*/
	int spacer = 4;
	float testFactor = 1.f;
	float progress;
	for (int i = 0; i < data.numPastPositions / spacer; ++i)
	{
		progress = 1.f - (i * spacer) / (float)MAX_PAST_POSITIONS;
		progress += .5;
		if (progress > 1.f)
			progress = 1.f;
		/*if (i * spacer > 45)
		{
			testFactor = .5;
		}
		else
		{
			testFactor = 1.f;
		}*/
		SetRectCenter(segmentQuads + i * 4, ts->tileWidth * progress, ts->tileHeight * progress, Vector2f(data.pastPositions[i * spacer]));
	}

	/*int i = 0;
	SetRectCenter(segmentQuads + 0 * 4, ts->tileWidth, ts->tileHeight, Vector2f(data.pastPositions[i]));
	i = 14;
	SetRectCenter(segmentQuads + 1 * 4, ts->tileWidth, ts->tileHeight, Vector2f(data.pastPositions[i]));
	i = 29;
	SetRectCenter(segmentQuads + 2 * 4, ts->tileWidth, ts->tileHeight, Vector2f(data.pastPositions[i]));
	i = 44;
	SetRectCenter(segmentQuads + 3 * 4, ts->tileWidth, ts->tileHeight, Vector2f(data.pastPositions[i]));
	i = 59;
	SetRectCenter(segmentQuads + 4 * 4, ts->tileWidth, ts->tileHeight, Vector2f(data.pastPositions[i]));*/


}

void Dragon::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(segmentQuads, data.numPastPositions / 4 * 4/*data.numPastPositions * 4*/, sf::Quads, ts->texture);

	DrawSprite(target, sprite);
}

int Dragon::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Dragon::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Dragon::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}