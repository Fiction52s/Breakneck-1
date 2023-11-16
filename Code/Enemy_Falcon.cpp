#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Falcon.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Falcon::Falcon(ActorParams *ap)
	:Enemy(EnemyType::EN_FALCON, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 7;
	actionLength[FLY] = 7;
	actionLength[RUSH] = 2;

	recoverDuration = 60;

	animFactor[NEUTRAL] = 5;
	animFactor[FLY] = 3;
	animFactor[RUSH] = 12;//8;

	rushSpeed = 30;


	accel.x = 1.0;//.6;
	accel.y = 2.0;//.7;//.3;

	maxSpeed.x = 20;
	maxSpeed.y = 10;

	ts = GetSizedTileset("Enemies/W4/falcon_256x256.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(9);
	cutObject->SetSubRectBack(10);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::ORANGE;

	//BasicCircleHurtBodySetup(40, 0, V2d( 20, 0 ), V2d());
	//BasicCircleHitBodySetup(40, 0, V2d(20, 0), V2d());
	BasicRectHitBodySetup(60, 20, 0, V2d(20, 15), V2d());
	BasicRectHurtBodySetup(60, 30, 0, V2d(20, 10), V2d());

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Falcon::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Falcon::SetLevel(int lev)
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

void Falcon::FlyMovement()
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

		if (data.velocity.x > maxSpeed.x )
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

void Falcon::ResetEnemy()
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

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	data.recoverFrame = 0;

	data.velocity = V2d();

	UpdateHitboxes();

	UpdateSprite();
}

void Falcon::ActionEnded()
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

void Falcon::FrameIncrement()
{
	if (action == FLY)
	{
		++data.recoverFrame;
	}
}

void Falcon::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			action = FLY;
			data.recoverFrame = 0;
			frame = 0;
			testOffsetDir = -dir;
		}
		break;
	case FLY:
		if (dist > DEFAULT_IGNORE_RADIUS)
		{
			action = NEUTRAL;
			frame = 0;
		}
		else if (data.recoverFrame == recoverDuration )
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
}

void Falcon::UpdateEnemyPhysics()
{
	if (action == FLY || action == RUSH )
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
}

void Falcon::UpdateHitboxes()
{
	V2d position = GetPosition();


	double ang = 0;// GetGroundedAngleRadians();

	if (action == RUSH)
	{
		ang = GetVectorAngleCW(data.velocity);
		if (!facingRight)
		{
			ang = ang + PI;
		}
	}
	//can update this with a universal angle at some point
	if (!hurtBody.IsEmpty())
	{
		hurtBody.SetBasicPos( currHurtboxFrame, position, ang);
		hurtBody.GetCollisionBoxes(currHurtboxFrame).at(0).flipHorizontal = !facingRight;
	}

	if (!hitBody.IsEmpty())
	{
		hitBody.SetBasicPos(currHitboxFrame, position, ang);
		hitBody.GetCollisionBoxes(currHitboxFrame).at(0).flipHorizontal = !facingRight;
	}

	auto comboBoxes = GetComboHitboxes();
	if (comboBoxes != NULL)
	{
		for (auto it = comboBoxes->begin(); it != comboBoxes->end(); ++it)
		{
			(*it).globalPosition = position;
		}
	}
}

void Falcon::UpdateSprite()
{

	int tile = 0;

	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		sprite.setRotation(0);
		break;
	case FLY:
		tile = frame / animFactor[FLY];
		sprite.setRotation(0);
		break;
	case RUSH:
		tile = frame / animFactor[RUSH] + 7;
		
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
	ts->SetSubRect(sprite, tile, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());


}

int Falcon::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Falcon::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Falcon::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}