#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Cheetah.h"

using namespace std;
using namespace sf;

Cheetah::Cheetah(ActorParams *ap)
	:Enemy(EnemyType::EN_CHEETAH, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	SetLevel(ap->GetLevel());

	maxGroundSpeed = 20;

	preChargeLimit = 40;

	turnaroundDist = 100;
	boostPastDist = 400;
	boostSpeed = 60;

	chargeLimit = 30;

	actionLength[IDLE] = 6;
	actionLength[CHARGE] = 8;
	actionLength[BOOST] = 1;
	actionLength[RUN] = 8;
	actionLength[LAND] = 1;
	actionLength[JUMP] = 2;

	animFactor[IDLE] = 16;
	animFactor[RUN] = 3;
	animFactor[CHARGE] = 3;
	animFactor[BOOST] = 30;
	animFactor[LAND] = 1;
	animFactor[JUMP] = 1;

	gravity = V2d(0, 1.2);//V2d(0, .6);

	jumpPower = V2d(12, -20);

	maxFallSpeed = 25;

	

	runAccel = 1.0;
	runDecel = 1.0;//runAccel * 3.0;

	CreateGroundMover(startPosInfo, 32, true, this);
	groundMover->AddAirForce(gravity);

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
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	if (PlayerDir().x >= 0)
	{
		facingRight = true;
	}
	else
	{
		facingRight = false;
	}

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	action = IDLE;
	frame = 0;

	data.preChargeFrame = 0;

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
			data.preChargeFrame = 0;
			frame = 0;
			break;
		case JUMP:
			frame = 1;
			break;
		case LAND:
			action = RUN;
			data.preChargeFrame = 0;
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

	double xDiff = PlayerDiff().x;
	double dist = PlayerDist();

	if (dead)
		return;
	
	ActionEnded();

	switch (action)
	{
	case IDLE:
		if (dist < DEFAULT_DETECT_RADIUS )//600)
		{
			action = RUN;
			frame = 0;
			data.preChargeFrame = 0;
		}
		break;
	case RUN:
	{
		if (dist >= DEFAULT_IGNORE_RADIUS)
		{
			action = IDLE;
			frame = 0;
		}
		else
		{
			if (data.preChargeFrame == preChargeLimit)
			{
				action = CHARGE;
				frame = 0;
				data.chargeFrame = 0;
			}
		}
		break;
	}
	case CHARGE:

		if (data.chargeFrame == chargeLimit)
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

		++data.preChargeFrame;
		break;
	}
	case JUMP:
	{
		if (facingRight)
		{
			groundMover->SetVelX(jumpPower.x);
		}
		else
		{
			groundMover->SetVelX(-jumpPower.x);
		}
		//cout << "jump: " << frame << endl;
		break;
	}
	case CHARGE:
		RunMovement();

		++data.chargeFrame;
		break;
	case BOOST:
		if (groundMover->GetGroundSpeed() > 0)
		{
			if (xDiff < -boostPastDist)
			{
				groundMover->SetSpeed(maxGroundSpeed);
				facingRight = false;
				action = RUN;
				data.preChargeFrame = 0;
				frame = 0;
			}
		}
		else if (groundMover->GetGroundSpeed() < 0)
		{
			if (xDiff > boostPastDist)
			{
				groundMover->SetSpeed(-maxGroundSpeed);
				facingRight = true;
				action = RUN;
				frame = 0;
				data.preChargeFrame = 0;
			}
		}

		
		break;
	}
}

void Cheetah::RunMovement()
{
	if (facingRight)
	{
		if (PlayerDiff().x < -turnaroundDist)
		{
			facingRight = false;
		}
	}
	else
	{
		if (PlayerDiff().x > turnaroundDist)
		{
			facingRight = true;
		}
	}

	if (facingRight) //clockwise
	{
		double accelFactor = runAccel;
		if (groundMover->GetGroundSpeed() < 0)
		{
			accelFactor = runDecel;
		}
		groundMover->SetSpeed(groundMover->GetGroundSpeed() + accelFactor);
	}
	else
	{
		double accelFactor = runAccel;
		if (groundMover->GetGroundSpeed() > 0)
		{
			accelFactor = runDecel;
		}
		groundMover->SetSpeed(groundMover->GetGroundSpeed() - accelFactor);
	}

	if (groundMover->GetGroundSpeed() > maxGroundSpeed)
		groundMover->SetSpeed(maxGroundSpeed);
	else if (groundMover->GetGroundSpeed() < -maxGroundSpeed)
		groundMover->SetSpeed(-maxGroundSpeed);
}

void Cheetah::UpdateEnemyPhysics()
{
	groundMover->Move(slowMultiple, numPhysSteps);

	if (groundMover->ground == NULL)
	{
		if (groundMover->GetVel().y > maxFallSpeed)
		{
			groundMover->SetVelY(maxFallSpeed);
		}
	}
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
	case JUMP:
		index = 0;
		break;
	case LAND:
		index = 0;
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
	if (action == RUN)
	{
		if ((facingRight && groundMover->GetGroundSpeed() < 0)
			|| (!facingRight && groundMover->GetGroundSpeed() > 0))
		{
			//cout << "here" << endl;
			groundMover->SetSpeed(0);
		}
		else if (facingRight && groundMover->GetGroundSpeed() > 0)
		{
			V2d v = jumpPower;
			groundMover->Jump(v);
			action = JUMP;
			frame = 0;
		}
		else if (!facingRight && groundMover->GetGroundSpeed() < 0)
		{
			V2d v = jumpPower;
			v.x = -v.x;
			groundMover->Jump(v);
			action = JUMP;
			frame = 0;
		}
	}
}

void Cheetah::ReachCliff()
{
	if (action == BOOST)
	{
		groundMover->SetSpeed(0);
		return;
	}

	//return;
	if (facingRight && groundMover->GetGroundSpeed() < 0
		|| !facingRight && groundMover->GetGroundSpeed() > 0)
	{
		groundMover->SetSpeed(0);
		return;
	}

	//cout << "reach cliff!" << endl;
	//ground = NULL;
	V2d v = jumpPower;
	if (!facingRight)
	{
		v.x = -v.x;
	}
	
	//action = LEDGEJUMP;
	action = JUMP;
	frame = 0;

	groundMover->Jump(v);
}

void Cheetah::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Cheetah::Land()
{
	action = LAND;
	frame = 0;

	//cout << "land" << endl;
}

int Cheetah::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void Cheetah::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void Cheetah::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}