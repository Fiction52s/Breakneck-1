#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Trailer.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Trailer::Trailer(ActorParams *ap)
	:Enemy(EnemyType::EN_TRAILER, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	actionLength[IDLE] = 1;
	actionLength[TRAIL] = 30;
	actionLength[PULSE] = 30;

	animFactor[IDLE] = 1;
	animFactor[TRAIL] = 1;
	animFactor[PULSE] = 1;

	pulseRadius = 150;
	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = .1;

	maxSpeed = 5;

	ts = GetSizedTileset("Enemies/W4/turtle_80x64.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(36);
	cutObject->SetSubRectBack(37);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	pulseBody.BasicCircleSetup(pulseRadius * scale, 0, V2d());
	pulseBody.hitboxInfo = hitboxInfo;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	trailMove = data.trailSeq.AddLineMovement(V2d(), V2d(), CubicBezier(0, 0, 1, 1),
		actionLength[TRAIL] * animFactor[TRAIL]);

	Color circleColor = Color::Red;
	circleColor.a = 100;
	testCircle.setFillColor(circleColor);
	testCircle.setRadius(pulseRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);

	ResetEnemy();
}

void Trailer::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Trailer::SetLevel(int lev)
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

void Trailer::ApproachMovement()
{
	if (PlayerDist() < 100)
	{
		data.velocity = V2d(0, 0);
	}
	else
	{
		data.velocity = PlayerDir() * maxSpeed;
	}
}

void Trailer::ResetEnemy()
{
	data.trailSeq.Reset();
	data.trailSeq.data.currMovement = NULL;

	action = IDLE;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Trailer::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case TRAIL:
			action = PULSE;
			break;
		case PULSE:
			DefaultHitboxesOn();
			StartTrail();
			break;
		}
	}
}

void Trailer::StartTrail()
{
	action = TRAIL;
	trailMove->start = GetPosition();
	trailMove->end = sess->GetPlayerPos();
	data.trailSeq.Reset();
}

void Trailer::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case IDLE:
		if (dist < attentionRadius)
		{
			action = TRAIL;
			frame = 0;
		}
		break;
	case TRAIL:
		if (dist > ignoreRadius)
		{
			action = IDLE;
			frame = 0;
		}
		break;
	case PULSE:
		break;
	}

	switch (action)
	{
	case IDLE:
		break;
	case TRAIL:
		break;
	case PULSE:
		break;
	}
}

void Trailer::UpdateEnemyPhysics()
{
	if (data.trailSeq.IsMovementActive())
	{
		data.trailSeq.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);
		currPosInfo.position = data.trailSeq.GetPos();
	}	
}

void Trailer::UpdateSprite()
{
	int trueFrame;

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());


	testCircle.setPosition(GetPositionF());
}

void Trailer::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (action == PULSE)
	{
		target->draw(testCircle);
	}
}