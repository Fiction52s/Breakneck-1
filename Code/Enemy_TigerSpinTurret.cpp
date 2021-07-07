#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_TigerSpinTurret.h"
#include "Actor.h"

using namespace std;
using namespace sf;


//TigerSpinTurretPool::TigerSpinTurretPool()
//{
//	ts = NULL;
//	numTurrets = 4;
//	turretVec.resize(numTurrets);
//	verts = new Vertex[numTurrets * 4];
//	for (int i = 0; i < numTurrets; ++i)
//	{
//		turretVec[i] = new TigerSpinTurret(verts + 4 * i);
//		turretVec[i]->pool = this;
//	}
//
//	Session *sess = Session::GetSession();
//
//	ts = sess->GetSizedTileset("Bosses/Bird/shuriken01_128x128.png");
//}
//
//TigerSpinTurretPool::~TigerSpinTurretPool()
//{
//	for (int i = 0; i < numTurrets; ++i)
//	{
//		delete turretVec[i];
//	}
//
//	delete[] verts;
//}
//
//void TigerSpinTurretPool::Reset()
//{
//	for (int i = 0; i < numTurrets; ++i)
//	{
//		turretVec[i]->Reset();
//	}
//}
//
//TigerSpinTurret * TigerSpinTurretPool::Throw(V2d &pos, V2d &dir)
//{
//	TigerSpinTurret *bs = NULL;
//	TigerSpinTurret *thrownShur = NULL;
//	for (int i = 0; i < numTurrets; ++i)
//	{
//		bs = turretVec[i];
//		if (!bs->spawned)
//		{
//			bs->Throw(pos, dir);
//			thrownShur = bs;
//			break;
//		}
//	}
//	return thrownShur;
//}
//
//void TigerSpinTurretPool::Draw(sf::RenderTarget *target)
//{
//	target->draw(verts, turretVec.size() * 4, sf::Quads, ts->texture);
//}

TigerSpinTurret::TigerSpinTurret(ActorParams *ap)
	:Enemy(EnemyType::EN_TIGERSPINTURRET, ap)
{
	SetNumActions(A_Count);
	SetEditorActions(HOMING, 0, 0);

	actionLength[HOMING] = 60;
	animFactor[HOMING] = 1;

	actionLength[TURRET] = 1;
	animFactor[TURRET] = 1;


	bulletSpeed = 10;
	framesBetween = 60;
	maxFramesToLive = 180 * 2;

	Tileset *ts_basicBullets = GetSizedTileset("Enemies/bullet_64x64.png");

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this, BasicBullet::BAT, 128, 2, GetPosition(),
		V2d(1, 0), 2 * PI, 40, false, 0, 0, ts_basicBullets);
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 18;

	ts = GetSizedTileset("Bosses/Tiger/bosstiger_spinturret_80x80.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(16);
	BasicCircleHitBodySetup(16);
	hitBody.hitboxInfo = hitboxInfo;

	ts_bulletExplode = GetTileset("FX/bullet_explode3_64x64.png");

	speed = 10;

	accel = .5;

	cutObject->Setup(ts, 0, 0, scale);
	//accel = .1;

	ResetEnemy();
}

void TigerSpinTurret::ResetEnemy()
{
	fireCounter = 0;
	facingRight = true;

	framesToLive = maxFramesToLive;
	velocity = initVel;

	action = HOMING;
	frame = 0;

	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void TigerSpinTurret::SetLevel(int lev)
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

void TigerSpinTurret::Init(V2d &pos, V2d &dir )
{
	startPosInfo.position = pos;
	initVel = dir * speed;
}

void TigerSpinTurret::BulletHitTerrain(BasicBullet *b, Edge *edge, V2d &pos)
{
	//V2d vel = b->velocity;
	//double angle = atan2( vel.y, vel.x );
	V2d norm = edge->Normal();
	double angle = atan2(norm.y, -norm.x);

	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, pos, true, -angle, 6, 2, true);
	b->launcher->DeactivateBullet(b);
}

void TigerSpinTurret::BulletHitPlayer(int playerIndex, BasicBullet *b, int hitResult)
{
	//if you dont deactivate the bullet it will hit constantly and make weird fx

	//cout << "hit player??" << endl;
	V2d vel = b->velocity;
	double angle = atan2(vel.y, vel.x);
	sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);

	if (hitResult != Actor::HitResult::INVINCIBLEHIT)
	{
		sess->PlayerApplyHit(playerIndex, b->launcher->hitboxInfo, NULL, hitResult, b->position);
	}

	b->launcher->DeactivateBullet(b);
}

void TigerSpinTurret::DirectKill()
{
	for (int i = 0; i < numLaunchers; ++i)
	{
		BasicBullet *b = launchers[0]->activeBullets;
		while (b != NULL)
		{
			BasicBullet *next = b->next;
			double angle = atan2(b->velocity.y, -b->velocity.x);
			sess->ActivateEffect(EffectLayer::IN_FRONT, ts_bulletExplode, b->position, true, angle, 6, 2, true);
			b->launcher->DeactivateBullet(b);

			b = next;
		}
	}
	receivedHit = NULL;
}

void TigerSpinTurret::FrameIncrement()
{
	if (action == TURRET)
	{
		++fireCounter;
	}

	if (framesToLive > 0)
	{
		--framesToLive;
		if (framesToLive == 0)
		{
			dead = true;
			sess->RemoveEnemy(this);
			//numHealth = 0;
		}
	}
}

void TigerSpinTurret::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();

	V2d diff = playerPos - position;
	V2d pDir = normalize(diff);

	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case HOMING:
			action = TURRET;
			facingDir = pDir;
			break;
		case TURRET:
			break;
		}
		frame = 0;
	}


	if (action == HOMING)
	{
		velocity += pDir * accel;
		CapVectorLength(velocity, speed);
	}
	else
	{
		velocity = V2d( 0, 0 );
	}

	if (slowCounter == 1 && action == TURRET)//&& action == FLY )
	{
		int f = fireCounter % 60;

		if (f % 5 == 0 )
		{
			launchers[0]->position = position;
			launchers[0]->facingDir = facingDir;
			launchers[0]->Fire();
			RotateCW(facingDir, PI / 32); //PI / 32);
		}
	}
	

	/*velocity += pDir * homingAccel;
	if (length(velocity) > homingSpeed)
	{
		velocity = normalize(velocity) * homingSpeed;
	}*/

	/*surfaceMover->velocity = pDir * unDodgeSpeed;
	unDodgeSpeed += unDodgeAccel;
	if (unDodgeSpeed > unDodgeMaxSpeed)
	{
		unDodgeSpeed = unDodgeMaxSpeed;
	}*/
}

void TigerSpinTurret::IHitPlayer(int index)
{
	/*if (action == THROWN)
	{
		if (shurType == SLIGHTHOMING || shurType == UNBLOCKABLE || shurType == UNDODGEABLE)
		{
			Die();
		}
	}
	else if (action == RETHROW)
	{
		if (shurType == SLIGHTHOMING_STICK
			|| shurType == UNBLOCKABLE_STICK
			|| shurType == UNDODGEABLE_STICK)
		{
			Die();
		}
	}*/
}

void TigerSpinTurret::UpdateEnemyPhysics()
{
	V2d movementVec = velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;

	//hitboxInfo->hitPosType = HitboxInfo::GetAirType(velocity);
}

void TigerSpinTurret::UpdateSprite()
{
	ts->SetSubRect(sprite, 0);
	sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void TigerSpinTurret::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}

void TigerSpinTurret::HandleHitAndSurvive()
{
	fireCounter = 0;
}

bool TigerSpinTurret::IsHitFacingRight()
{
	return velocity.x > 0;
}