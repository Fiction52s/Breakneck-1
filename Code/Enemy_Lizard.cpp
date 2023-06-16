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

	scale = 1.25;

	SetNumActions(Count);
	SetEditorActions(IDLE, 0, 0);

	actionLength[IDLE] = 11 * 5;
	actionLength[LAND] = 1;
	actionLength[JUMP] = 2;
	actionLength[RUN] = 16;
	//actionLength[JUMPSQ]
	actionLength[SHOCK] = 20;

	animFactor[IDLE] = 1;
	animFactor[LAND] = 1;
	animFactor[JUMP] = 1;
	animFactor[RUN] = 3;
	animFactor[SHOCK] = 1;

	gravity = .5;
	maxGroundSpeed = 10;
	maxFallSpeed = 20;
	runAccel = .3;//1.0;
	runDecel = .3;//runAccel * 3.0;
	fireWaitDuration = 60;
	bulletClockwise = true;

	CreateGroundMover(startPosInfo, 40, true, this);
	groundMover->AddAirForce(V2d(0, gravity));
	groundMover->SetSpeed(0);

	ts = GetSizedTileset("Enemies/W5/lizard_192x96.png");
	ts_bulletExplode = GetSizedTileset("FX/bullet_explode2_64x64.png");

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this,
		BasicBullet::LIZARD, 32, 1, GetPosition(), V2d(0, -1), 0, 180, true);
	launchers[0]->SetBulletSpeed(10);
	launchers[0]->hitboxInfo->hType = HitboxInfo::RED;
	launchers[0]->hitboxInfo->damage = 60;

	SetOffGroundHeight(128 / 2);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 12;
	hitboxInfo->hType = HitboxInfo::RED;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);
	hitBody.hitboxInfo = hitboxInfo;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(24);
	cutObject->SetSubRectBack(25);
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
	groundMover->DebugDraw(target);
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

	data.fireWaitCounter = 0;

	frame = 0;

	UpdateSprite();
	UpdateHitboxes();
}

void Lizard::ActionEnded()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case RUN:
			frame = 0;
			break;
		case JUMP:
			frame = 1;
			break;
		case LAND:
		{
			action = RUN;
			frame = 0;
			break;
		}
		case SHOCK:
		{
			action = RUN;
			frame = 0;
			data.fireWaitCounter = 0;
			break;
		}
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
		if (dist < DEFAULT_DETECT_RADIUS)
		{
			action = RUN;
			frame = 0;
		}
		break;
	}

	case RUN:
	{
		double dist = length(playerPos - position);
		if (dist >= DEFAULT_IGNORE_RADIUS)
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
		break;
	case JUMP:
		if (facingRight)
		{
			groundMover->SetVelX(maxGroundSpeed);
		}
		else
		{
			groundMover->SetVelX(-maxGroundSpeed);
		}
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

	assert(data.fireWaitCounter <= fireWaitDuration);
	if (action == RUN && data.fireWaitCounter == fireWaitDuration && slowCounter == 1)
	{
		
		//ground has to be not null.
		launchers[0]->position = GetPosition();

		action = SHOCK;
		frame = 0;

		groundMover->SetSpeed(0);
		
		if (groundMover->ground != NULL)
		{
			launchers[0]->facingDir = -groundMover->ground->Normal();
		}
		else
		{
			launchers[0]->facingDir = V2d(0, 1);
		}
		
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
			if (groundMover->GetVel().y > maxFallSpeed)
			{
				groundMover->SetVelY(maxFallSpeed);
			}
			else if (groundMover->GetVel().y < -maxFallSpeed)
			{
				groundMover->SetVelY(-maxFallSpeed);
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
	gb->data.clockwise = bulletClockwise;
}

void Lizard::FrameIncrement()
{
	if (action == RUN )
	{
		data.fireWaitCounter++;
		/*if (data.fireWaitCounter == fireWaitDuration)
		{
			data.fireWaitCounter = 0;
		}*/
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
	int tile = 0;
	switch (action)
	{
	case IDLE:
	{
		tile = 0;
		break;
	}
	case RUN:
	{
		tile = frame / animFactor[RUN];
		break;
	}
	case JUMP:
	{
		if (groundMover->GetVel().y > 0)
		{
			tile = 19;
		}
		else
		{
			tile = 18;
		}
		break;
	}
	case LAND:
	{
		tile = 20;
		break;
	}
	case SHOCK:
	{
		tile = 16;
		break;
	}
	}
	IntRect r = ts->GetSubRect(tile);
	//if (!facingRight)
	if (facingRight)
	{
		r = sf::IntRect(r.left + r.width, r.top, -r.width, r.height);
	}

	sprite.setTextureRect(r);


	int extraVert = 43;
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height - extraVert);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(groundMover->GetAngleDegrees());
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
		if ((facingRight && groundMover->GetGroundSpeed() < 0)
			|| (!facingRight && groundMover->GetGroundSpeed() > 0))
		{
			//cout << "here" << endl;
			groundMover->SetSpeed(0);
		}
		else if (facingRight && groundMover->GetGroundSpeed() > 0)
		{
			V2d v = V2d(maxGroundSpeed, -10);
			groundMover->Jump(v);
			action = JUMP;
			frame = 0;
		}
		else if (!facingRight && groundMover->GetGroundSpeed() < 0)
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
	if ((facingRight && groundMover->GetGroundSpeed() < 0)
		|| (!facingRight && groundMover->GetGroundSpeed() > 0))
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
	V2d &pos)
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
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[i]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}

	Enemy::DirectKill();
}

int Lizard::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void Lizard::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}

void Lizard::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}
