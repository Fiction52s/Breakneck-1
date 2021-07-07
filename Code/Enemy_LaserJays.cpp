#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_LaserJays.h"
#include "Actor.h"

using namespace std;
using namespace sf;

LaserJays::LaserJays(ActorParams *ap)
	:Enemy(EnemyType::EN_LASERJAYS, ap)
{
	secondary = false;
	Construct(ap);
}

LaserJays::LaserJays(ActorParams *ap, bool second)
	:Enemy(EnemyType::EN_LASERJAYS, ap)
{
	secondary = second;
	Construct(ap);
}

void LaserJays::Construct(ActorParams *ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 1;
	actionLength[APPROACH] = 2;
	actionLength[CHARGE] = 40;
	actionLength[PULSE] = 20;
	actionLength[RECOVER] = 30;

	animFactor[NEUTRAL] = 1;
	animFactor[APPROACH] = 1;
	animFactor[PULSE] = 1;
	animFactor[RECOVER] = 1;

	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = 1.0;//.3;//.1;

	maxSpeed = 20;//5;

	activatePulseRadius = 300;

	ts = GetSizedTileset("Enemies/W6/laserjays_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	otherJay = NULL;
	otherParams = NULL;
	
	cutObject->SetTileset(ts);

	if (secondary)
	{
		cutObject->SetSubRectFront(1);
		cutObject->SetSubRectBack(1);
	}
	else
	{
		cutObject->SetSubRectFront(0);
		cutObject->SetSubRectBack(0);
	}
	
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->kbDir = V2d(1, 0);
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	laserBody.BasicCircleSetup(10 * scale, 0, V2d());
	laserBody.hitboxInfo = hitboxInfo;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	TryCreateSecondary(ap);

	ResetEnemy();
}

LaserJays::~LaserJays()
{
	if (!secondary)
	{
		delete otherParams;
	}
}

void LaserJays::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void LaserJays::AddToGame()
{
	Enemy::AddToGame();
	if (!secondary)
	{
		otherJay->AddToGame();
	}
}

void LaserJays::TryCreateSecondary(ActorParams *ap)
{
	assert(otherJay == NULL);
	if (!secondary)
	{
		otherParams = ap->Copy();
		otherJay = new LaserJays(otherParams, true);
		otherJay->otherJay = this;
		otherParams->myEnemy = otherJay;
	}
}

void LaserJays::SetLevel(int lev)
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

void LaserJays::ApproachMovement()
{
	if (PlayerDist() < 100)
	{
		velocity = V2d(0, 0);
	}
	else
	{
		V2d offset(0, -300);
		if (secondary)
		{
			offset.y = -offset.y;
		}
		V2d targetDir = PlayerDir(V2d(), V2d(offset));
		velocity += targetDir * accel;
		CapVectorLength(velocity, maxSpeed);
	}
}

void LaserJays::ResetEnemy()
{
	/*if (GetPosition().x < sess->playerOrigPos[0].x)
	facingRight = false;
	else
	facingRight = true;*/

	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();

	if (!secondary)
	{
		otherJay->Reset();
		//otherJay->SetCurrPosInfo(startPosInfo);
	}
}

void LaserJays::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	if (!secondary)
	{
		otherJay->UpdateOnPlacement(ap); //assuming that ap is editparams here
	}
}

void LaserJays::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case APPROACH:
			break;
		case CHARGE:
			action = PULSE;
			SetHitboxes(&laserBody, 0);
			break;
		case PULSE:
			DefaultHitboxesOn();
			action = RECOVER;
			break;
		case RECOVER:
			action = APPROACH;
			frame = 0;
			break;
		}
	}
}

void LaserJays::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		if (dist < attentionRadius)
		{
			action = APPROACH;
			frame = 0;
		}
		break;
	case APPROACH:
		if (dist < activatePulseRadius)
		{
			action = CHARGE;
			frame = 0;
		}
		else if (dist > ignoreRadius)
		{
			action = NEUTRAL;
			frame = 0;
		}
		break;
	case CHARGE:
		break;
	case PULSE:
		break;
	case RECOVER:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		velocity = V2d(0, 0);
		break;
	case APPROACH:
		ApproachMovement();
		break;
	case CHARGE:
		ApproachMovement();
		break;
	case PULSE:
		ApproachMovement();
		break;
	case RECOVER:
		ApproachMovement();
		break;
	}
}

void LaserJays::UpdateEnemyPhysics()
{
	if (action == APPROACH || action == CHARGE || action == PULSE || action == RECOVER)
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;

		laserBody.SetBasicPos(currPosInfo.position);
	}
}

void LaserJays::UpdateSprite()
{
	/*int trueFrame;
	switch (action)
	{
	case NEUTRAL:
		sprite.setColor(Color::White);
		break;
	case APPROACH:
		sprite.setColor(Color::White);
		break;
	case CHARGE:
		sprite.setColor(Color::Green);
		break;
	case PULSE:
		sprite.setColor(Color::White);
		break;
	case RECOVER:
		sprite.setColor(Color::Blue);

		break;
	}*/

	int tile = 0;
	if (secondary)
	{
		tile = 1;
	}

	ts->SetSubRect(sprite, tile, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	if (!secondary)
	{
		V2d diff = otherJay->GetPosition() - GetPosition();
		V2d laserDir = normalize(diff);
		double laserAngle = GetVectorAngleCW(laserDir);

		SetRectRotation(laserQuad, laserAngle, length(diff), 10, 
			Vector2f(GetPosition() + diff / 2.0));
		SetRectColor(laserQuad, Color::Magenta);
	}
	
}

void LaserJays::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (!secondary)
	{
		target->draw(laserQuad, 4, sf::Quads);
	}
}