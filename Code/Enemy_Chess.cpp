#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Chess.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Chess::Chess(ActorParams *ap)
	:Enemy(EnemyType::EN_CHESS, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 1;
	actionLength[RUSH] = 2;

	/*animFactor[NEUTRAL] = 1;
	animFactor[APPROACH] = 1;
	animFactor[PULSE] = 1;
	animFactor[RECOVER] = 1;*/

	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = .1;

	maxSpeed = 5;

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
	circleColor.a = 100;
	testCircle.setFillColor(circleColor);
	testCircle.setRadius(50);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);

	ResetEnemy();
}

void Chess::UpdatePreFrameCalculations()
{
	Actor *targetPlayer = sess->GetPlayer(0);

	sess->ForwardSimulatePlayer(0, 30);
	testCircle.setPosition(Vector2f(sess->GetPlayerPos(0)));
	//targetPos = sess->GetPlayerPos(pIndex);
	sess->RevertSimulatedPlayer(0);
	
	//predictCircle.setPosition(Vector2f(targetPos));
	//hasPredictedPos = true;

}

void Chess::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Chess::SetLevel(int lev)
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

void Chess::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void Chess::ActionEnded()
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
			action = NEUTRAL;
			break;
		}
	}
}

void Chess::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		if (dist < attentionRadius)
		{
			action = NEUTRAL;
			frame = 0;
		}
		break;
	case RUSH:
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
	case RUSH:
		break;
	}
}

void Chess::UpdateEnemyPhysics()
{
	if (action == RUSH )
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;
	}
}

void Chess::UpdateSprite()
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
	}

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());


	//testCircle.setPosition(GetPositionF());
}

void Chess::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	target->draw(testCircle);
}