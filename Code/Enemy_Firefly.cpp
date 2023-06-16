#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Firefly.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Firefly::Firefly(ActorParams *ap)
	:Enemy(EnemyType::EN_FIREFLY, ap)
{
	RegisterCollisionBody(data.pulseBody);

	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 4;
	actionLength[APPROACH] = 4;
	actionLength[CHARGE] = 40;
	actionLength[PULSE] = 20;
	actionLength[RECOVER] = 30;

	animFactor[NEUTRAL] = 5;
	animFactor[APPROACH] = 5;
	animFactor[CHARGE] = 1;
	animFactor[PULSE] = 3;
	animFactor[RECOVER] = 1;

	pulseRadius = 150;
	
	accel = .1;

	maxSpeed = 5;

	activatePulseRadius = 300;

	ts = GetSizedTileset("Enemies/W3/pulser_192x192.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(21);
	cutObject->SetSubRectBack(20);
	cutObject->SetScale(scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 3;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;
	hitboxInfo->hType = HitboxInfo::YELLOW;

	data.pulseBody.BasicCircleSetup(pulseRadius * scale, 0, V2d());
	data.pulseBody.hitboxInfo = hitboxInfo;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	Color circleColor = Color::Red;
	circleColor.a = 100;
	testCircle.setFillColor(circleColor);
	testCircle.setRadius(pulseRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);

	ts->SetSpriteTexture(pulseSprite);
	ts->SetSubRect(pulseSprite, 0);
	pulseSprite.setOrigin(pulseSprite.getLocalBounds().width / 2, pulseSprite.getLocalBounds().height / 2);
	pulseSprite.setScale(2, 2);


	ResetEnemy();
}

void Firefly::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void Firefly::SetLevel(int lev)
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

void Firefly::ApproachMovement()
{
	if (PlayerDist() < 100)
	{
		velocity = V2d(0, 0);
	}
	else
	{
		velocity = PlayerDir() * maxSpeed;
	}
}

void Firefly::ResetEnemy()
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

void Firefly::ActionEnded()
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
			SetHitboxes(&data.pulseBody, 0);
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

void Firefly::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		if (dist < DEFAULT_DETECT_RADIUS)
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
		else if(dist > DEFAULT_IGNORE_RADIUS)
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

void Firefly::UpdateEnemyPhysics()
{
	if (action == APPROACH || action == CHARGE || action == PULSE || action == RECOVER)
	{
		V2d movementVec = velocity;
		movementVec /= slowMultiple * (double)numPhysSteps;

		currPosInfo.position += movementVec;

		data.pulseBody.SetBasicPos(currPosInfo.position);
	}
}

void Firefly::UpdateSprite()
{
	int trueFrame = 0;
	switch (action)
	{
	case NEUTRAL:
		//sprite.setColor(Color::White);
		trueFrame = frame / animFactor[NEUTRAL];
		break;
	case APPROACH:
		trueFrame = frame / animFactor[APPROACH];
		//sprite.setColor(Color::White);
		break;
	case CHARGE:
		trueFrame = ((frame / animFactor[NEUTRAL]) % actionLength[NEUTRAL]) + 10;
		//trueFrame = frame / animFactor[APPROACH];
		//sprite.setColor(Color::Green);
		break;
	case PULSE:
	{
		trueFrame = ((frame / animFactor[NEUTRAL]) % actionLength[NEUTRAL]) + 5;
		int f = (frame / 2) % 10;
		if( f > 7 )
			f = 7;
		ts->SetSubRect(pulseSprite, 22 + f);//27);
		//sprite.setColor(Color::White);
		break;
	}
	case RECOVER:
		trueFrame = ((frame / animFactor[NEUTRAL]) % actionLength[NEUTRAL]) + 15;
		//sprite.setColor(Color::Blue);
		break;
	}

	ts->SetSubRect(sprite, trueFrame, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());

	pulseSprite.setPosition(GetPositionF());
	testCircle.setPosition(GetPositionF());
}

void Firefly::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (action == PULSE)
	{
		target->draw(pulseSprite);
		//target->draw(testCircle);
	}
}

int Firefly::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Firefly::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Firefly::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}