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

	actionLength[NEUTRAL] = 1;
	actionLength[FLY] = 2;

	animFactor[NEUTRAL] = 1;
	animFactor[FLY] = 1;


	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = 1.0;

	maxSpeed = 30;

	ts = sess->GetSizedTileset("Enemies/turtle_80x64.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(36);
	cutObject->SetSubRectBack(37);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	ResetEnemy();
}

void Falcon::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
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
	if (PlayerDist() < 100)
	{
		velocity = V2d(0, 0);
	}
	else
	{
		velocity += PlayerDir() * accel;
		CapVectorLength(velocity, maxSpeed);
	}
}

void Falcon::ResetEnemy()
{

	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

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
		}
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
		if (dist < attentionRadius)
		{
			action = FLY;
			frame = 0;
		}
		break;
	case FLY:
		if (dist > ignoreRadius)
		{
			action = NEUTRAL;
			frame = 0;
		}
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		velocity = V2d(0, 0);
		break;
	case FLY:
		FlyMovement();
		break;
	}
}

void Falcon::UpdateEnemyPhysics()
{
	if (action == FLY )
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
}

void Falcon::UpdateSprite()
{
	int trueFrame;
	switch (action)
	{
	case NEUTRAL:
		sprite.setColor(Color::White);
		break;

		break;
	}

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Falcon::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}