#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Roadrunner.h"
#include "Shield.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )
#define COLOR_GREEN Color( 0, 0xcc, 0x44 )
#define COLOR_YELLOW Color( 0xff, 0xf0, 0 )
#define COLOR_ORANGE Color( 0xff, 0xbb, 0 )
#define COLOR_RED Color( 0xff, 0x22, 0 )
#define COLOR_MAGENTA Color( 0xff, 0, 0xff )
#define COLOR_WHITE Color( 0xff, 0xff, 0xff )

Roadrunner::Roadrunner(ActorParams *ap)
	:Enemy(EnemyType::EN_ROADRUNNER, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 11 * 5;
	actionLength[LAND] = 1;
	actionLength[JUMP] = 2;
	actionLength[RUN] = 3;//9 * 4;
	actionLength[WAKEUP] = 30;

	animFactor[RUN] = 5;

	gravity = .5;
	maxGroundSpeed = 30;
	maxFallSpeed = 20;
	runAccel = 1.0;
	runDecel = runAccel * 3.0;

	attentionRadius = 800;//800;
	ignoreRadius = 2000;

	CreateGroundMover(startPosInfo, 40, true, this);
	groundMover->AddAirForce(V2d(0, gravity));
	groundMover->SetSpeed(0);

	ts = GetSizedTileset("Enemies/W3/roadrunner_256x256.png");

	SetOffGroundHeight(128 / 2);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->hType = HitboxInfo::YELLOW;



	//BasicRectHurtBodySetup(30, 50, startPosInfo.GetGroundAngleRadians(), V2d(0, 50), GetPosition());
	//BasicRectHitBodySetup(30, 50, startPosInfo.GetGroundAngleRadians(), V2d(0, 50), GetPosition());

	BasicCircleHurtBodySetup(48, 0, V2d( 0, -30), GetPosition());
	BasicCircleHitBodySetup(48, 0, V2d(0, -30), GetPosition());
	hitBody.hitboxInfo = hitboxInfo;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(0);
	cutObject->SetScale(scale);

	ResetEnemy();
}

void Roadrunner::SetLevel(int lev)
{
	level = lev;

	switch (level)
	{
	case 1:
		scale = .5;
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

void Roadrunner::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	//if (!dead)
	//testMover->physBody.DebugDraw(target);
}

void Roadrunner::ResetEnemy()
{
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	action = IDLE;

	facingRight = true;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}

void Roadrunner::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case WAKEUP:
			action = RUN;
			frame = 0;
			break;
		case RUN:
			frame = 0;
			break;
		case JUMP:
			frame = 1;
			break;
		case LAND:
			action = RUN;
			frame = 0;
			break;
		}
	}
}

void Roadrunner::ProcessState()
{
	//cout << "vel: " << testMover->velocity.x << ", " << testMover->velocity.y << endl;
	//Actor *player = owner->GetPlayer( 0 );
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	double dist = PlayerDist();

	ActionEnded();

	switch (action)
	{
	case IDLE:
	{
		if (dist < attentionRadius)
		{
			action = RUN;
			frame = 0;
		}
		break;
	}
	
	case RUN:
	{
		double dist = length(playerPos - position);
		if (dist >= ignoreRadius)
		{
			action = IDLE;
			frame = 0;
		}
		break;
	}
	
	case JUMP:
		break;
		//case ATTACK:
		//	break;
	case LAND:
		break;
	}

	switch (action)
	{
	case IDLE:
		groundMover->SetSpeed(0);
		//cout << "idle: " << frame << endl;
		break;
	case RUN:
		if (facingRight)
		{
			if (playerPos.x < position.x - 50)
			{
				facingRight = false;
			}
		}
		else
		{
			if (playerPos.x > position.x + 50)
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
		break;
	case JUMP:
		//cout << "jump: " << frame << endl;
		break;
		//	case ATTACK:
		//	{
		//		testMover->SetSpeed( 0 );
		//	}
		//	break;
	case LAND:
	{
		//	cout << "land: " << frame << endl;
		//testMover->SetSpeed( 0 );
	}
	break;
	default:
		//cout << "WAATATET" << endl;
		break;
	}
}

void Roadrunner::UpdateEnemyPhysics()
{
	if (numHealth > 0) //!dead
	{
		groundMover->Move(slowMultiple, numPhysSteps);

		if (groundMover->ground == NULL)
		{
			if (groundMover->velocity.y > maxFallSpeed)
			{
				groundMover->velocity.y = maxFallSpeed;
			}
			else if (groundMover->velocity.y < -maxFallSpeed)
			{
				groundMover->velocity.y = -maxFallSpeed;
			}
		}


		V2d gn(0, -1);
		if (groundMover->ground != NULL)
		{
			gn = groundMover->ground->Normal();
		}
	}
}


void Roadrunner::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}


void Roadrunner::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}



void Roadrunner::UpdateSprite()
{

	int tile = 0;
	switch (action)
	{
	case RUN:
	{
		tile = frame / animFactor[RUN];
		break;
	}
	case JUMP:
	{
		tile = 1;
		break;
	}
	}

	IntRect r = ts->GetSubRect(tile);
	if (!facingRight )
	{
		r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
	}

	sprite.setTextureRect(r);


	int extraVert = 64;
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(groundMover->GetAngleDegrees());
}

void Roadrunner::HitTerrain(double &q)
{
	//cout << "hit terrain? " << endl;
}

bool Roadrunner::StartRoll()
{
	return false;
}

void Roadrunner::FinishedRoll()
{

}

void Roadrunner::HitOther()
{
	//cout << "hit other" << endl;

	if (action == RUN)
	{
		if ((facingRight && groundMover->groundSpeed < 0)
			|| (!facingRight && groundMover->groundSpeed > 0))
		{
			//cout << "here" << endl;
			groundMover->SetSpeed(0);
		}
		else if (facingRight && groundMover->groundSpeed > 0)
		{
			V2d v = V2d(maxGroundSpeed, -10);
			groundMover->Jump(v);
			action = JUMP;
			frame = 0;
		}
		else if (!facingRight && groundMover->groundSpeed < 0)
		{
			V2d v = V2d(-maxGroundSpeed, -10);
			groundMover->Jump(v);
			action = JUMP;
			frame = 0;
		}
	}
}

void Roadrunner::ReachCliff()
{
	if ((facingRight && groundMover->groundSpeed < 0)
		|| (!facingRight && groundMover->groundSpeed > 0))
	{
		groundMover->SetSpeed(0);
		return;
	}

	double jumpStrength = -10;

	V2d v;
	if (facingRight)
	{
		v = V2d(maxGroundSpeed, jumpStrength);
	}
	else
	{
		v = V2d(-maxGroundSpeed, jumpStrength);
	}

	groundMover->Jump(v);

	action = JUMP;
	frame = 0;
}

void Roadrunner::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Roadrunner::Land()
{
	action = LAND;
	frame = 0;
}