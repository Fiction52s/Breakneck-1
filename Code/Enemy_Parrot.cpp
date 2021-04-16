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

	actionLength[NEUTRAL] = 1;
	actionLength[FLY] = 2;

	animFactor[NEUTRAL] = 1;
	animFactor[FLY] = 1;

	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = 1.0;

	maxSpeed = 15;

	ts = sess->GetSizedTileset("Enemies/W4/turtle_80x64.png");
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
	hitboxInfo->kbDir = V2d(1, 0);

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 12, 1, GetPosition(), V2d(1, 0), 0, 90, false);
	launchers[0]->SetBulletSpeed(15);
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

	fireCounter = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Parrot::FrameIncrement()
{
	if (action != NEUTRAL)
	{
		++fireCounter;
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
		velocity += PlayerDir(V2d(), V2d( 0, -300 )) * accel;
		CapVectorLength(velocity, maxSpeed);
		break;
	}

	if (action != NEUTRAL)
	{
		if (fireCounter == 30)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = PlayerDir();
			launchers[0]->Fire();
			fireCounter = 0;
		}
	}
}

void Parrot::UpdateEnemyPhysics()
{
	if (action == FLY )
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
}

void Parrot::UpdateSprite()
{

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void Parrot::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void Parrot::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	b->launcher->DeactivateBullet(b);
}

void Parrot::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}

void Parrot::DirectKill()
{
	BasicBullet *b = launchers[0]->activeBullets;
	while (b != NULL)
	{
		BasicBullet *next = b->next;
		double angle = atan2(b->velocity.y, -b->velocity.x);
		//sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
		b->launcher->DeactivateBullet(b);

		b = next;
	}

	receivedHit = NULL;
}