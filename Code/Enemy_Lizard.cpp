#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_Lizard.h"
#include "Shield.h"
#include "Actor.h"

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

Lizard::Lizard(ActorParams *ap)
	:Enemy(EnemyType::EN_LIZARD, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 11 * 5;
	actionLength[LAND] = 1;
	actionLength[JUMP] = 2;
	actionLength[RUN] = 9 * 4;
	actionLength[WAKEUP] = 30;

	gravity = .5;
	maxGroundSpeed = 2;//30;
	maxFallSpeed = 20;
	runAccel = 1.0;
	runDecel = runAccel * 3.0;
	fireWaitDuration = 60;
	bulletClockwise = true;

	attentionRadius = 800;//800;
	ignoreRadius = 2000;

	CreateGroundMover(startPosInfo, 40, true, this);
	groundMover->AddAirForce(V2d(0, gravity));
	groundMover->SetSpeed(0);

	ts = sess->GetSizedTileset("Enemies/Roadrunner_256x256.png");
	ts_bulletExplode = sess->GetSizedTileset("FX/bullet_explode2_64x64.png");

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this,
		BasicBullet::LIZARD, 32, 1, GetPosition(), V2d(0, -1), 0, 180, true);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->damage = 18;

	SetOffGroundHeight(128 / 2);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 0;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);
	hitBody.hitboxInfo = hitboxInfo;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(0);
	cutObject->SetSubRectBack(0);
	cutObject->SetScale(scale);

	ResetEnemy();
}

void Lizard::SetLevel(int lev)
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

void Lizard::DebugDraw(RenderTarget *target)
{
	Enemy::DebugDraw(target);
	//if (!dead)
	//testMover->physBody.DebugDraw(target);
}

void Lizard::ResetEnemy()
{
	groundMover->Set(startPosInfo);
	groundMover->SetSpeed(0);

	action = IDLE;

	facingRight = true;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	fireWaitCounter = 0;

	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}

void Lizard::UpdateHitboxes()
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

void Lizard::ActionEnded()
{
	if (frame == actionLength[action])
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

void Lizard::ProcessState()
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

	if (action != IDLE && fireWaitCounter == 0 && slowCounter == 1)
	{
		//ground has to be not null.
		launchers[0]->position = GetPosition();
		V2d norm = groundMover->ground->Normal();
		launchers[0]->facingDir = -norm;//PlayerDir();
		bulletClockwise = true;
		launchers[0]->Fire();
		bulletClockwise = false;
		launchers[0]->Fire();
	}
}

void Lizard::UpdateEnemyPhysics()
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

void Lizard::FireResponse( BasicBullet *b )
{
	GrindBullet *gb = (GrindBullet*)b;
	gb->clockwise = bulletClockwise;
}

void Lizard::FrameIncrement()
{
	if (action != IDLE)
	{
		fireWaitCounter++;
		if (fireWaitCounter == fireWaitDuration)
		{
			fireWaitCounter = 0;
		}
	}
}

void Lizard::HandleNoHealth()
{
	cutObject->SetFlipHoriz(!facingRight);
	cutObject->rotateAngle = sprite.getRotation();
}


void Lizard::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}



void Lizard::UpdateSprite()
{

	IntRect r = ts->GetSubRect(0);
	if (!facingRight)
	{
		r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
	}

	sprite.setTextureRect(r);


	int extraVert = 64;
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(groundMover->GetAngleDegrees());


	SyncSpriteInfo(auraSprite, sprite);
}

void Lizard::HitTerrain(double &q)
{
	//cout << "hit terrain? " << endl;
}

bool Lizard::StartRoll()
{
	return false;
}

void Lizard::FinishedRoll()
{

}

void Lizard::HitOther()
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

void Lizard::ReachCliff()
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

void Lizard::HitOtherAerial(Edge *e)
{
	//cout << "hit edge" << endl;
}

void Lizard::Land()
{
	action = LAND;
	frame = 0;
}

void Lizard::BulletHitTerrain(BasicBullet *b,
	Edge *edge,
	sf::Vector2<double> &pos)
{
	/*V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);*/
}

void Lizard::BulletHitPlayer(
	int playerIndex,
	BasicBullet *b,
	int hitResult)
{
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
}

void Lizard::UpdateBullet(BasicBullet *b)
{

}

void Lizard::DirectKill()
{

}