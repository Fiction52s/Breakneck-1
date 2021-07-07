#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Cheetah.h"

using namespace std;
using namespace sf;

Cheetah::Cheetah(ActorParams *ap)
	:Enemy(EnemyType::EN_CHEETAH, ap), moveBezTest(.22, .85, .3, .91)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	SetLevel(ap->GetLevel());

	maxGroundSpeed = 20;
	jumpStrength = 5;

	preChargeLimit = 40;

	turnaroundDist = 100;
	boostPastDist = 400;
	boostSpeed = 60;

	chargeFrames = 30;

	actionLength[IDLE] = 6;
	actionLength[CHARGE] = 8;
	actionLength[BOOST] = 1;
	actionLength[RUN] = 8;

	animFactor[IDLE] = 16;
	animFactor[RUN] = 3;
	animFactor[CHARGE] = 3;
	animFactor[BOOST] = 30;

	gravity = V2d(0, .6);

	maxFallSpeed = 25;

	attentionRadius = 800;
	ignoreRadius = 2000;

	

	runAccel = 1.0;
	runDecel = 1.0;//runAccel * 3.0;

	CreateGroundMover(startPosInfo, 32, true, this);
	groundMover->AddAirForce(V2d(0, .6));

	ts = GetSizedTileset("Enemies/W4/cheetah_224x96.png");

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 15;
	hitboxInfo->hType = HitboxInfo::ORANGE;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	cutObject->Setup(ts, 23, 24, scale);

	bezLength = 60 * NUM_STEPS;

	ResetEnemy();
}

void Cheetah::SetLevel(int lev)
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

//void Cheetah::HandleNoHealth()
//{
//	cutObject->SetFlipHoriz(facingRight);
//	cutObject->rotateAngle = sprite.getRotation();
//}

void Cheetah::ResetEnemy()
{
	facingRight = true;
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	bezFrame = 0;
	attackFrame = -1;

	action = IDLE;
	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}


void Cheetah::ActionEnded()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case CHARGE:
			break;
		case BOOST:
			action = RUN;
			preChargeFrames = 0;
			frame = 0;
			break;
		}
	}
}

void Cheetah::Jump(double strengthx, double strengthy)
{
	V2d jumpVec = V2d(strengthx, -strengthy);
	groundMover->Jump(jumpVec);
}

void Cheetah::ProcessState()
{

	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	double xDiff = PlayerDistX();
	double dist = PlayerDist();

	if (dead)
		return;
	
	ActionEnded();

	switch (action)
	{
	case IDLE:
		if (dist < attentionRadius)
		{
			action = RUN;
			frame = 0;
			preChargeFrames = 0;
		}
		break;
	case RUN:
	{
		if (dist >= ignoreRadius)
		{
			action = IDLE;
			frame = 0;
		}
		else
		{
			if (preChargeFrames == preChargeLimit)
			{
				action = CHARGE;
				frame = 0;
				chargeCounter = 0;
			}
		}
		break;
	}
	case CHARGE:

		if (chargeFrames == chargeCounter)
		{
			action = BOOST;
			frame = 0;

			if (facingRight)
			{
				groundMover->SetSpeed(boostSpeed);
			}
			else
			{
				groundMover->SetSpeed(-boostSpeed);
			}
		}
		break;
	case BOOST:
		
		break;
	}

	switch (action)
	{
	case IDLE:
		groundMover->SetSpeed(0);
		break;
	case RUN:
	{
		RunMovement();

		++preChargeFrames;
		break;
	}
	case CHARGE:
		RunMovement();

		++chargeCounter;
		break;
	case BOOST:
		if (groundMover->groundSpeed > 0)
		{
			if (xDiff < -boostPastDist)
			{
				groundMover->SetSpeed(maxGroundSpeed);
				facingRight = false;
				action = RUN;
				preChargeFrames = 0;
				frame = 0;
			}
		}
		else if (groundMover->groundSpeed < 0)
		{
			if (xDiff > boostPastDist)
			{
				groundMover->SetSpeed(-maxGroundSpeed);
				facingRight = true;
				action = RUN;
				frame = 0;
				preChargeFrames = 0;
			}
		}

		
		break;
	}
}

void Cheetah::RunMovement()
{
	if (facingRight)
	{
		if (PlayerDistX() < -turnaroundDist)
		{
			facingRight = false;
		}
	}
	else
	{
		if (PlayerDistX() > turnaroundDist)
		{
			facingRight = true;
		}
	}

	if (facingRight) //clockwise
	{
		double accelFactor = runAccel;
		if (groundMover->groundSpeed < 0)
		{
			accelFactor = runDecel;
		}
		groundMover->SetSpeed(groundMover->groundSpeed + accelFactor);
	}
	else
	{
		double accelFactor = runAccel;
		if (groundMover->groundSpeed > 0)
		{
			accelFactor = runDecel;
		}
		groundMover->SetSpeed(groundMover->groundSpeed - accelFactor);
	}

	if (groundMover->groundSpeed > maxGroundSpeed)
		groundMover->SetSpeed(maxGroundSpeed);
	else if (groundMover->groundSpeed < -maxGroundSpeed)
		groundMover->SetSpeed(-maxGroundSpeed);
}

void Cheetah::UpdateEnemyPhysics()
{
	groundMover->Move(slowMultiple, numPhysSteps);

	if (groundMover->ground == NULL)
	{
		if (groundMover->velocity.y > maxFallSpeed)
		{
			groundMover->velocity.y = maxFallSpeed;
		}
	}
}

void Cheetah::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}


void Cheetah::UpdateSprite()
{
	int airRange = 3;
	int fallRange = 15;

	int index = 0;
	switch (action)
	{
	case RUN:
	{
		index = frame / animFactor[RUN];
		break;
	}
	case IDLE:
		index = frame / animFactor[IDLE] + 16;
		break;
	case CHARGE:
		index = frame / animFactor[CHARGE] + 8;
		break;
	case BOOST:
		index = 22;//frame / animFactor[BOOST] + 22;
		break;
	}

	ts->SetSubRect(sprite, index, !facingRight, false);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(groundMover->GetAngleDegrees());
	//sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

bool Cheetah::StartRoll()
{
	return false;
}

void Cheetah::FinishedRoll()
{

}

void Cheetah::HitOther()
{
	/*V2d v;
	if( facingRight && mover->groundSpeed > 0 )
	{
	v = V2d( 10, -10 );
	mover->Jump( v );
	}
	else if( !facingRight && mover->groundSpeed < 0 )
	{
	v = V2d( -10, -10 );
	mover->Jump( v );
	}*/
	//cout << "hit other!" << endl;
	//mover->SetSpeed( 0 );
	//facingRight = !facingRight;
}

void Cheetah::ReachCliff()
{
	return;
	if (facingRight && groundMover->groundSpeed < 0
		|| !facingRight && groundMover->groundSpeed > 0)
	{
		groundMover->SetSpeed(0);
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;
	V2d v;
	if (facingRight)
	{
		v = V2d(10, -10);
	}
	else
	{
		v = V2d(-10, -10);
	}

	//action = LEDGEJUMP;
	frame = 0;

	Jump(v.x, v.y);
}

void Cheetah::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Cheetah::Land()
{
	return;
	//action = LAND;
	frame = 0;

	//cout << "land" << endl;
}