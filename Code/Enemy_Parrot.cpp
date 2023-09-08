#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Parrot.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Parrot::Parrot(ActorParams *ap)
	:Enemy(EnemyType::EN_PARROT, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 10;
	actionLength[FLY] = 10;
	actionLength[ATTACK] = 9;
	

	animFactor[NEUTRAL] = 8;
	animFactor[FLY] = 4;
	animFactor[ATTACK] = 4;

	accel = 1.0;

	maxSpeed = 15;

	ts = GetSizedTileset("Enemies/W5/parrot_256x256.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(20);
	cutObject->SetSubRectBack(19);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::RED;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 12, 1, GetPosition(), V2d(1, 0), 0, 90, false);
	launchers[0]->SetBulletSpeed(15);
	launchers[0]->hitboxInfo->hType = HitboxInfo::RED;
	launchers[0]->Reset();

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Parrot::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Parrot::SetLevel(int lev)
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

void Parrot::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	data.velocity = V2d();
	data.fireCounter = 0;

	
	
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

	UpdateHitboxes();

	UpdateSprite();
}

void Parrot::FrameIncrement()
{
	if (action != NEUTRAL && action != ATTACK )
	{
		++data.fireCounter;
	}
	
}

void Parrot::ActionEnded()
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
		case ATTACK:
			action = FLY;
			frame = 0;
			break;
		}
	}
}

void Parrot::ProcessState()
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
			frame = 0;
		}
		break;
	case FLY:
		if (dist > DEFAULT_IGNORE_RADIUS)
		{
			action = NEUTRAL;
			frame = 0;
		}
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		data.velocity = V2d(0, 0);
		break;
	case FLY:
	case ATTACK:
		data.velocity += PlayerDir(V2d(), V2d( 0, -300 )) * accel;
		CapVectorLength(data.velocity, maxSpeed);
		break;
	}

	if (action == FLY || action == ATTACK)
	{
		if (dir.x >= 0)
		{
			facingRight = true;
		}
		else
		{
			facingRight = false;
		}
	}

	int throwFrame = 3;
	if (action == ATTACK && frame == throwFrame * animFactor[ATTACK] && slowCounter == 1 )
	{
		V2d shootOffset(70, 20);
		if (!facingRight)
		{
			shootOffset.x = -shootOffset.x;
		}
		launchers[0]->position = GetPosition() + shootOffset;
		launchers[0]->facingDir = PlayerDir();
		launchers[0]->Fire();
	}

	if (action == FLY)
	{
		if (data.fireCounter == 30)
		{
			action = ATTACK;
			frame = 0;
			data.fireCounter = 0;
		}
	}

	
}

void Parrot::UpdateEnemyPhysics()
{
	if (action == FLY || action == ATTACK)
	{
		V2d movementVec = data.velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
}

void Parrot::UpdateSprite()
{
	int tile = 0;
	switch (action)
	{
	case NEUTRAL:
		tile = frame / animFactor[NEUTRAL];
		break;
	case FLY:
		tile = frame / animFactor[FLY];
		break;
	case ATTACK:
		tile = frame / animFactor[ATTACK] + 10;
		break;
	}
	ts->SetSubRect(sprite, tile, facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

int Parrot::GetNumStoredBytes()
{
	return sizeof(MyData) + GetNumStoredLauncherBytes();
}

void Parrot::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	StoreBytesForLaunchers(bytes);
	bytes += GetNumStoredLauncherBytes();
}

void Parrot::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	SetLaunchersFromBytes(bytes);
	bytes += GetNumStoredLauncherBytes();
}