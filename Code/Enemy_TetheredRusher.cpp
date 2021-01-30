#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_TetheredRusher.h"
#include "Actor.h"

using namespace std;
using namespace sf;

TetheredRusher::TetheredRusher(ActorParams *ap)
	:Enemy(EnemyType::EN_TETHEREDRUSHER, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 1;
	actionLength[RUSH] = 30;
	actionLength[RECOVER] = 30;

	animFactor[NEUTRAL] = 1;
	animFactor[RUSH] = 1;
	animFactor[RECOVER] = 1;

	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = 10.0;

	maxSpeed = 50;

	chainRadius = 400;

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

	Color circleColor = Color::Red;
	circleColor.a = 40;
	testCircle.setFillColor(circleColor);
	testCircle.setRadius(chainRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);

	ms.AddLineMovement(V2d(), V2d(), CubicBezier(), actionLength[RUSH] * animFactor[RUSH]);
	ms.AddLineMovement(V2d(), V2d(), CubicBezier(), actionLength[RECOVER] * animFactor[RECOVER]);

	ResetEnemy();
}

void TetheredRusher::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void TetheredRusher::SetLevel(int lev)
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

void TetheredRusher::ResetEnemy()
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
}

void TetheredRusher::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		case RUSH:
			action = RECOVER;
			break;
		case RECOVER:
			action = NEUTRAL;
			break;
		}
	}
}

void TetheredRusher::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		if (dist < attentionRadius)
		{
			action = RUSH;
			frame = 0;
			ms.Reset();
		}
		break;
	case RUSH:
		break;
	case RECOVER:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		velocity = V2d(0, 0);
		break;
	case RUSH:
		break;
	case RECOVER:
		break;
	}
}

void TetheredRusher::UpdateEnemyPhysics()
{
	if (action == RUSH || action == RECOVER)
	{
		ms.Update(slowMultiple, NUM_MAX_STEPS / numPhysSteps);
	}
}

void TetheredRusher::UpdateSprite()
{
	int trueFrame;
	switch (action)
	{
	case NEUTRAL:
		sprite.setColor(Color::White);
		break;
	case RUSH:
		sprite.setColor(Color::Green);
		break;
	case RECOVER:
		sprite.setColor(Color::Blue);
		break;
	}

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());


	testCircle.setPosition(GetPositionF());
}

void TetheredRusher::EnemyDraw(sf::RenderTarget *target)
{
	target->draw(testCircle);

	DrawSprite(target, sprite);

	
}