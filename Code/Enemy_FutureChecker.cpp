#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_FutureChecker.h"
#include "Actor.h"

using namespace std;
using namespace sf;

const int FutureChecker::predictFrames = 25;

FutureChecker::FutureChecker(ActorParams *ap)
	:Enemy(EnemyType::EN_FUTURECHECKER, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	bulletSpeed = 5;

	action = NEUTRAL;

	actionLength[NEUTRAL] = 1;
	actionLength[FIRE] = 15;
	actionLength[FADEIN] = 4;//60;
	actionLength[FADEOUT] = 17;//90;
	actionLength[INVISIBLE] = 15;

	animFactor[NEUTRAL] = 2;
	animFactor[FIRE] = 2;
	animFactor[FADEIN] = 7;
	animFactor[FADEOUT] = 2;
	animFactor[INVISIBLE] = 1;

	accel = 2.0;
	maxSpeed = 10.0;

	fireCounter = 0;

	ts = sess->GetSizedTileset("Enemies/W4/turtle_80x64.png");
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
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 8;
	hitboxInfo->kbDir = V2d(1, 0);

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = sess->GetSizedTileset("FX/bullet_explode3_64x64.png");

	ResetEnemy();
}

void FutureChecker::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	//cutObject->SetCutRootPos(Vector2f(position));
}

void FutureChecker::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = 4.0;
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

void FutureChecker::ResetEnemy()
{
	if (GetPosition().x < sess->playerOrigPos[0].x)
		facingRight = false;
	else
		facingRight = true;
	fireCounter = 0;

	action = NEUTRAL;
	frame = 0;

	velocity = V2d();

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

int FutureChecker::GetNumSimulationFramesRequired()
{
	return predictFrames + 2;
}

void FutureChecker::ActionEnded()
{
	int blah = actionLength[action] * animFactor[action];
	//cout << "frame: " << frame << ", actionlength: " << blah << endl;
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case NEUTRAL:
			frame = 0;
			break;
		case FIRE:
			action = FADEOUT;
			velocity = V2d();
			frame = 0;
			break;
		case INVISIBLE:
			currPosInfo.position = sess->GetFuturePlayerPos( 25 );
			if (playerPos.x < GetPosition().x)
			{
				facingRight = false;
			}
			else
			{
				facingRight = true;
			}
			action = FADEIN;
			frame = 0;
			break;
		case FADEIN:
			//action = FIRE;
			action = FADEOUT;
			velocity = V2d();
			frame = 0;
			break;
		case FADEOUT:
			velocity = V2d();
			action = INVISIBLE;
			frame = 0;
			SetHitboxes(NULL, 0);
			SetHurtboxes(NULL, 0);
			playerTrackPos = playerPos;
			break;
		}
	}
}

void FutureChecker::ProcessState()
{
	ActionEnded();

	switch (action)
	{
	case NEUTRAL:
		if (action == NEUTRAL)
		{
			if (PlayerDist() <= 1000)
			{
				action = FADEOUT;
				frame = 0;
			}
		}
		//cout << "NEUTRAL";
		break;
	case FIRE:
		//cout << "FIRE";
		break;
	case INVISIBLE:
		//cout << "INVISIBLE";
		break;
	case FADEIN:
		//cout << "FADEIN";
		break;
	case FADEOUT:
		//cout << "FADEOUT";
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
	case FIRE:
		break;
	case INVISIBLE:
		break;
	case FADEIN:
		if (frame == 5 && slowCounter == 1)
		{
			DefaultHitboxesOn();
			DefaultHurtboxesOn();
			//velocity = PlayerDir() * 5.0;
		}
		break;
	case FADEOUT:
		if (PlayerDist() > 30)
		{
			velocity = PlayerDir() * maxSpeed;
		}
		//velocity += PlayerDir() * accel;
		//CapVectorLength(velocity, maxSpeed);
		break;
	}
}

void FutureChecker::UpdateEnemyPhysics()
{
	V2d movementVec = velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;

}

void FutureChecker::UpdateSprite()
{
	int trueFrame;
	switch (action)
	{
	case NEUTRAL:
		trueFrame = 0;
		break;
	case FIRE:
		trueFrame = frame / animFactor[FIRE] + 21;
		break;
	case INVISIBLE:
		return;
		break;
	case FADEIN:
		trueFrame = frame / animFactor[FADEIN] + 17;
		break;
	case FADEOUT:
		trueFrame = frame / animFactor[FADEOUT];
		break;
	}

	//cout << "trueFrame: " << trueFrame << ", action: " << action << endl;
	IntRect ir = ts->GetSubRect(trueFrame);
	if (!facingRight)
	{
		ir.left += ir.width;
		ir.width = -ir.width;
	}

	//sprite.setScale( 2, 2 );
	sprite.setTextureRect(ir);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void FutureChecker::EnemyDraw(sf::RenderTarget *target)
{
	if (action != INVISIBLE)
		DrawSprite(target, sprite);
}