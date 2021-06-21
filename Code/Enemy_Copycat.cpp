#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Copycat.h"
#include "Actor.h"

using namespace std;
using namespace sf;

Copycat::Copycat(ActorParams *ap)
	:Enemy(EnemyType::EN_COPYCAT, ap), moveBezTest(.22, .85, .3, .91)
{
	SetNumActions(A_Count);
	SetEditorActions(IDLE, IDLE, 0);

	SetLevel(ap->GetLevel());

	maxGroundSpeed = 20;
	jumpStrength = 5;

	preChargeLimit = 40;

	turnaroundDist = 100;
	boostPastDist = 400;
	boostSpeed = 100;

	actionLength[IDLE] = 10;
	actionLength[STAND] = 10;
	actionLength[RUN] = 5;
	actionLength[JUMP] = 1;
	actionLength[DOUBLEJUMP] = 1;

	animFactor[IDLE] = 1;

	gravity = V2d(0, 1.0);

	airAccel = 3.0;

	maxFallSpeed = 25;

	attentionRadius = 800;
	ignoreRadius = 2000;

	player = sess->GetPlayer(0);


	runAccel = 1.0;
	runDecel = 1.0;//runAccel * 3.0;

	CreateGroundMover(startPosInfo, 32, true, this);
	groundMover->AddAirForce(V2d(0, .6));

	ts = sess->GetSizedTileset("Enemies/W3/Badger_192x128.png");

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHitBodySetup(32);
	BasicCircleHurtBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(33);
	cutObject->SetSubRectBack(34);
	cutObject->SetScale(scale);

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::TURTLE, 12, 1, GetPosition(), V2d(1, 0), 0, 90, false);
	launchers[0]->SetBulletSpeed(15);
	launchers[0]->Reset();

	bezLength = 60 * NUM_STEPS;

	ResetEnemy();
}

void Copycat::SetLevel(int lev)
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

void Copycat::HandleNoHealth()
{
	cutObject->SetFlipHoriz(facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}

void Copycat::ResetEnemy()
{
	hasDoubleJump = true;

	fireCounter = 0;
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

void Copycat::UpdateHitboxes()
{
	Edge *ground = groundMover->ground;
	if (ground != NULL)
	{
		V2d knockbackDir(1, -1);
		knockbackDir = normalize(knockbackDir);
		if (groundMover->groundSpeed > 0)
		{
			hitboxInfo->kbDir = knockbackDir;
			hitboxInfo->knockback = 15;
		}
		else
		{
			hitboxInfo->kbDir = V2d(-knockbackDir.x, knockbackDir.y);
			hitboxInfo->knockback = 15;
		}
	}
	else
	{
		//hitBody.globalAngle = 0;
		//hurtBody.globalAngle = 0;
	}

	BasicUpdateHitboxes();
}

void Copycat::ActionEnded()
{
	if (frame == animFactor[action] * actionLength[action])
	{
		frame = 0;
		switch (action)
		{
		case IDLE:
			break;
		case RUN:
			break;
		}
	}
}

void Copycat::Jump(double strengthx, double strengthy)
{
	V2d jumpVec = V2d(strengthx, -strengthy);
	groundMover->Jump(jumpVec);
}

void Copycat::ProcessState()
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
	case STAND:
	{
		if (TryJump())
		{

		}
		else if (HoldingLeft())
		{
			facingRight = false;
			action = RUN;
			frame = 0;
		}
		else if (HoldingRight())
		{
			facingRight = true;
			action = RUN;
			frame = 0;
		}
		break;
	}
	case RUN:
	{
		if (dist >= ignoreRadius)
		{
			action = IDLE;
			frame = 0;
		}

		else if (TryJump())
		{

		}
		else if (HoldingLeft())
		{
			if (facingRight)
			{
				facingRight = false;
			}
		}
		else if (HoldingRight())
		{
			if (!facingRight)
			{
				facingRight = true;
			}
		}
		else
		{
			action = STAND;
			frame = 0;
		}
		break;
	}
	case JUMP:
	{
		if (TryDoubleJump())
		{

		}
		break;
	}
	case DOUBLEJUMP:
		break;
	}

	switch (action)
	{
	case IDLE:
		groundMover->SetSpeed(0);
		break;
	case STAND:
		groundMover->SetSpeed(0);
		break;
	case RUN:
	{
		RunMovement();

		++preChargeFrames;
		break;
	}
	case DOUBLEJUMP:
	case JUMP:
	{
		if (HoldingLeft())
		{
			groundMover->velocity.x += -runAccel;
		}
		else if (HoldingRight())
		{
			groundMover->velocity.x += runAccel;
		}

		CapQuantityAbs(groundMover->velocity.x, maxGroundSpeed);
		break;
	}
	}

	if (action != IDLE)
	{
		if (fireCounter == 30)
		{
			launchers[0]->position = GetPosition();
			launchers[0]->facingDir = PlayerDir();
			launchers[0]->Fire();
			fireCounter = 0;
		}
		else
		{
			++fireCounter;
		}
	}
	
}

bool Copycat::TryJump()
{
	
	if (player->JumpButtonPressed())
	{
		action = JUMP;
		hasDoubleJump = true;
		frame = 0;

		if (HoldingLeft())
		{
			facingRight = false;
			//Jump(groundMover->groundSpeed, 20);
		}
		else if (HoldingRight())
		{
			facingRight = true;
			//Jump(groundMover->groundSpeed, 20);
		}
		else
		{
			//Jump(0, 20);
		}

		Jump(groundMover->groundSpeed, 20);

		return true;
	}

	return false;
}

bool Copycat::TryDoubleJump()
{
	if ( hasDoubleJump && player->JumpButtonPressed() 
		&& groundMover->ground == NULL )
	{
		action = DOUBLEJUMP;
		frame = 0;

		hasDoubleJump = false;

		if (HoldingLeft())
		{
			facingRight = false;
			//Jump(groundMover->groundSpeed, 20);
		}
		else if (HoldingRight())
		{
			facingRight = true;
			//Jump(groundMover->groundSpeed, 20);
		}
		else
		{
			groundMover->velocity.x = 0;
			//Jump(0, 20);
		}

		Jump(groundMover->velocity.x, 20);

		return true;
	}

	return false;
}

void Copycat::RunMovement()
{
	/*if (facingRight)
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
	}*/

	if (player->currInput.LRight()) //clockwise
	{
		double accelFactor = runAccel;
		if (groundMover->groundSpeed < 0)
		{
			accelFactor = runDecel;
		}
		groundMover->SetSpeed(groundMover->groundSpeed + accelFactor);
	}
	else if(player->currInput.LLeft())
	{
		double accelFactor = runAccel;
		if (groundMover->groundSpeed > 0)
		{
			accelFactor = runDecel;
		}
		groundMover->SetSpeed(groundMover->groundSpeed - accelFactor);
	}

	CapQuantityAbs(groundMover->groundSpeed, maxGroundSpeed);

}

void Copycat::UpdateEnemyPhysics()
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

void Copycat::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite );
}


void Copycat::UpdateSprite()
{
	int airRange = 3;
	int fallRange = 15;

	int index = 0;
	switch (action)
	{
	case IDLE:
		index = 0;
		break;
	case STAND:
		break;
	case RUN:
		index = 0;
		break;
	}

	ts->SetSubRect(sprite, index, !facingRight, false);

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

bool Copycat::StartRoll()
{
	return false;
}

void Copycat::FinishedRoll()
{

}

void Copycat::HitOther()
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

void Copycat::ReachCliff()
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

void Copycat::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Copycat::Land()
{
	if (HoldingLeft())
	{
		action = RUN;
		facingRight = false;
	}
	else if (HoldingRight())
	{
		action = RUN;
		facingRight = true;
	}
	else
	{
		action = STAND;
	}

	hasDoubleJump = true;


	groundMover->groundSpeed = groundMover->velocity.x;

	//action = LAND;
	frame = 0;

	//cout << "land" << endl;
}

bool Copycat::HoldingLeft()
{
	return player->currInput.LLeft();
}

bool Copycat::HoldingRight()
{
	return player->currInput.LRight();
}

void Copycat::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	b->launcher->DeactivateBullet(b);
}

void Copycat::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}
}

void Copycat::DirectKill()
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