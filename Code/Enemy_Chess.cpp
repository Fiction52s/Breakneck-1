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
	actionLength[RUSH] = 60;
	actionLength[RECOVER] = 60;
	actionLength[CHASE] = 2;

	/*animFactor[NEUTRAL] = 1;
	animFactor[APPROACH] = 1;
	animFactor[PULSE] = 1;
	animFactor[RECOVER] = 1;*/

	attentionRadius = 800;
	ignoreRadius = 2000;

	accel = 1.0;

	maxSpeed = 20;

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
			StartRush();
			//action = RECOVER;
			break;
		case RECOVER:
			//StartRush();
			break;
		}
	}
}

void Chess::StartRush()
{
	action = RUSH;
	frame = 0;

	int predictFrame = 30;
	V2d futurePos = sess->GetFuturePlayerPos(predictFrame);

	V2d rushDir = normalize(futurePos - GetPosition());

	double rushDist = length(futurePos - GetPosition());
	double speed = rushDist / predictFrame;
	double time = rushDist / speed;
	actionLength[RUSH] = time + 1;//time * 1.5;

	testCircle.setPosition(Vector2f(futurePos));

	velocity = rushDir * speed;
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
			action = CHASE;
			frame = 0;
			//StartRush();
		}
		break;
	case RUSH:
		/*if (dist > ignoreRadius)
		{
			action = NEUTRAL;
			frame = 0;
		}*/
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		velocity = V2d(0, 0);
		break;
	case RUSH:
		break;
	case CHASE:
	{
		V2d futurePos = sess->GetFuturePlayerPos(20);
		V2d newPos;
		newPos.x = GetPosition().x;
		newPos.y = futurePos.y;
		testCircle.setPosition(Vector2f(newPos));

		currPosInfo.position = newPos;
		velocity = V2d();
		//V2d futureDir = normalize(futurePos - GetPosition());
		//velocity = 20.0 * futureDir;//+= futureDir * accel;
		//CapVectorLength(velocity, maxSpeed);
		break;
	}
		
	}
}

void Chess::UpdateEnemyPhysics()
{
	if (action == RUSH || action == CHASE)
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
	case RECOVER:
		sprite.setColor(Color::Black);
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