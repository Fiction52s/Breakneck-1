#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_ExplodingBarrel.h"
#include "Eye.h"
#include "AbsorbParticles.h"
#include "MovingGeo.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

ExplodingBarrel::ExplodingBarrel(ActorParams *ap)
	:Enemy(EnemyType::EN_EXPLODINGBARREL, ap)//, false, 1, false)
{
	RegisterCollisionBody(explosion);

	SetNumActions(S_Count);
	SetEditorActions(S_CHARGE, S_CHARGE, 0);

	actionLength[S_IDLE] = 15;
	actionLength[S_TINYCHARGE] = 3;
	actionLength[S_CHARGE] = 12;
	actionLength[S_ABOUT_TO_EXPLODE] = 60;
	actionLength[S_EXPLODE] = 20;

	animFactor[S_IDLE] = 1;
	animFactor[S_CHARGE] = 5;
	animFactor[S_TINYCHARGE] = 5;
	animFactor[S_ABOUT_TO_EXPLODE] = 1;
	animFactor[S_EXPLODE] = 1;


	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	facingRight = true;

	ts = GetSizedTileset("Enemies/W3/barrel_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	//SetOffGroundHeight(ts->tileHeight / 2.0 - 20);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 180;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;//20;//4;

	explosionInfo.damage = 180;
	explosionInfo.drainX = 0;
	explosionInfo.drainY = 0;
	explosionInfo.hitlagFrames = 0;
	explosionInfo.hitstunFrames = 15;
	explosionInfo.knockback = 20;
	explosionInfo.hitPosType = HitboxInfo::HitPosType::OMNI;
	//hitboxInfo->hitPosType = HitboxInfo::HitPosType::OMNI;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);
	hitBody.hitboxInfo = hitboxInfo;

	comboObj = new ComboObject(this);

	double explosionRadius = 400;//300;

	//explosion.ResetFrames();
	explosion.BasicCircleSetup(explosionRadius * scale, 0, V2d());
	explosion.hitboxInfo = &explosionInfo;//hitboxInfo;

	comboObj->enemyHitboxInfo = new HitboxInfo;
	comboObj->enemyHitboxInfo->damage = 20;
	comboObj->enemyHitboxInfo->drainX = .5;
	comboObj->enemyHitboxInfo->drainY = .5;
	comboObj->enemyHitboxInfo->hitlagFrames = 0;
	comboObj->enemyHitboxInfo->hitstunFrames = 30;
	comboObj->enemyHitboxInfo->knockback = 0;
	comboObj->enemyHitboxInfo->freezeDuringStun = true;
	comboObj->enemyHitboxInfo->hType = HitboxInfo::COMBO;
	comboObj->enemyHitboxInfo->hitPosType = HitboxInfo::HitPosType::OMNI;

	Color exploColor = Color::Red;
	exploColor.a = 100;
	testCircle.setFillColor(exploColor);
	testCircle.setRadius(explosionRadius);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);
	//comboObj->enemyHitboxInfo->hDir = receivedHit->hDir;

	comboObj->enemyHitBody.BasicCircleSetup(explosionRadius, GetPosition());

	ResetEnemy();
}

ExplodingBarrel::~ExplodingBarrel()
{
}

void ExplodingBarrel::SetLevel(int lev)
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

void ExplodingBarrel::ResetEnemy()
{
	comboObj->Reset();

	testCircle.setPosition(GetPositionF());

	explosion.SetBasicPos(GetPosition());

	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	action = S_IDLE;
	frame = 0;

	UpdateHitboxes();

	UpdateSprite();
}

void ExplodingBarrel::ProcessHit()
{
	if (numHealth == 1)
	{
		if (!dead && HasReceivedHit() && numHealth > 0)
		{
			//sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());
			//ConfirmKill();
			sess->PlayerConfirmEnemyKill(this, GetReceivedHitPlayerIndex());

			HitboxInfo::HitboxType hType;
			if (!receivedHit.IsEmpty())
			{
				hType = receivedHit.hType;
			}
			else
			{
				hType = HitboxInfo::HitboxType::NORMAL;
			}
			if (hType == HitboxInfo::COMBO)
			{
				pauseFrames = 7;
				Enemy *ce = sess->GetEnemyFromID(comboHitEnemyID);
				ce->ComboKill(this);
			}
			else if (hType == HitboxInfo::WIREHITRED || hType == HitboxInfo::WIREHITBLUE)
			{
				pauseFrames = 7;
			}
			else
			{
				pauseFrames = 7;
				//sess->Pause(7);
				//pauseFrames = 0;
			}
			pauseBeganThisFrame = true;
			pauseFramesFromAttacking = false;

			PlayDeathSound();

			if (hasMonitor && !suppressMonitor)
			{
				suppressMonitor = true;
				sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::DARK,
					sess->GetPlayer(receivedHitPlayerIndex), GetNumDarkAbsorbParticles(), GetPosition());
			}
			else
			{
				//sess->ActivateAbsorbParticles(AbsorbParticles::AbsorbType::ENERGY,
				//	sess->GetPlayer(receivedHitPlayerIndex), GetNumEnergyAbsorbParticles(), GetPosition());
			}

			action = S_ABOUT_TO_EXPLODE;
			frame = 0;
			//--numHealth;
			HitboxesOff();
			HurtboxesOff();

			receivedHit.SetEmpty();
		}
	}
	else
	{
		Enemy::ProcessHit();
	}
	
}

void ExplodingBarrel::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_EXPLODE:
			numHealth = 0;
			dead = true;
			sess->PlayerRemoveActiveComboer(comboObj);
			break;
		case S_IDLE:
			if (PlayerDist() > 300)
			{
				action = S_CHARGE;
			}
			else
			{
				action = S_TINYCHARGE;
			}

			break;
		case S_ABOUT_TO_EXPLODE:
		{

			action = S_EXPLODE;
			frame = 0;
			SetHitboxes(&explosion, 0);
			sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());

			sess->cam.SetRumble(1.5, 1.5, 7);

			break;
		}
		case S_TINYCHARGE:
		case S_CHARGE:
		{
			action = S_IDLE;
			break;
		}
		}
	}
}

void ExplodingBarrel::HandleNoHealth()
{

}

void ExplodingBarrel::UpdateEnemyPhysics()
{
}

void ExplodingBarrel::FrameIncrement()
{
}

void ExplodingBarrel::ComboHit()
{
	pauseFrames = 5;
	/*++currHits;
	if (currHits >= hitLimit)
	{
		action = S_EXPLODE;
		frame = 0;
	}*/
}

void ExplodingBarrel::UpdateSprite()
{
	sprite.setPosition(GetPositionF());
	int tile = 0;
	switch (action)
	{
	case S_IDLE:
		tile = 0;
		break;
	case S_CHARGE:
	{
		tile = frame / animFactor[S_CHARGE];
		break;
	}
	case S_TINYCHARGE:
	{
		tile = frame / animFactor[S_TINYCHARGE] + 1;
		break;
	}
	case S_ABOUT_TO_EXPLODE:
	{
		tile = 0;
		break;
	}
		
		
	}

	if (action == S_ABOUT_TO_EXPLODE)
	{

		sprite.setColor(GetBlendColor(Color::Yellow, Color::Red, (float)frame / (actionLength[S_ABOUT_TO_EXPLODE] * animFactor[S_ABOUT_TO_EXPLODE])));
	}
	else
	{
		sprite.setColor(Color::White);
	}

	sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	//sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

void ExplodingBarrel::DirectKill()
{
	Enemy::DirectKill();
	sess->PlayerRemoveActiveComboer(comboObj);
}

void ExplodingBarrel::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (action == S_EXPLODE)
	{
		target->draw(testCircle);
	}
}

int ExplodingBarrel::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void ExplodingBarrel::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);
}

void ExplodingBarrel::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);
}