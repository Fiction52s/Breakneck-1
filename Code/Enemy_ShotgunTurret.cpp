#include "Enemy.h"

#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_ShotgunTurret.h"
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



ShotgunTurret::ShotgunTurret(ActorParams *ap)
	:Enemy(EnemyType::EN_SHOTGUNTURRET, ap)
{
	SetNumActions(Count);
	SetEditorActions(ATTACK, 0, 0);

	SetLevel(ap->GetLevel());

	bulletSpeed = 3;

	actionLength[IDLE] = 9;
	actionLength[ATTACK] = 14;// * 3;//120;
	
	
	animFactor[IDLE] = 10;
	animFactor[ATTACK] = 3;

	actionLength[WAIT] = 120 - actionLength[ATTACK] * animFactor[ATTACK];

	animFactor[WAIT] = 1;

	ts = GetSizedTileset("Enemies/W3/cactus_160x160.png");

	double width = ts->tileWidth;
	double height = ts->tileHeight - 24;

	width *= scale;
	height *= scale;

	SetOffGroundHeight(height / 2.f);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	//shield = new Shield(Shield::ShieldType::T_BLOCK, 80 * scale, 3, this);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 4;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->hType = HitboxInfo::YELLOW;

	BasicCircleHurtBodySetup(40);
	BasicCircleHitBodySetup(40);
	hitBody.hitboxInfo = hitboxInfo;

	bulletSpeed = 7;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this,
		BasicBullet::CACTUS, 32, 3, GetPosition(), V2d(0, -1),
		PI / 6, 180, false);
	/*launchers[0] = new Launcher(this,
		BasicBullet::SHOTGUN, 32, 1, GetPosition(), V2d(0, -1),
		0, 180, false);*/
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 18;
	launchers[0]->hitboxInfo->hType = HitboxInfo::YELLOW;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(23);
	cutObject->SetSubRectBack(24);
	cutObject->SetScale(scale);
	cutObject->rotateAngle = sprite.getRotation();

	UpdateOnPlacement(ap);


	ResetEnemy();
}

void ShotgunTurret::UpdateOnPlacement(ActorParams *ap)
{
	Enemy::UpdateOnPlacement(ap);

	if (startPosInfo.ground != NULL)
	{
		launchers[0]->position = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
		launchers[0]->facingDir = startPosInfo.GetEdge()->Normal();
	}

	//testShield->SetPosition(GetPosition());
}

void ShotgunTurret::ResetEnemy()
{
	action = IDLE;
	frame = 0;
	DefaultHurtboxesOn();
	DefaultHitboxesOn();
	//currShield = shield;
	//shield->Reset();

	UpdateHitboxes();
	UpdateSprite();
}

void ShotgunTurret::SetLevel(int lev)
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

void ShotgunTurret::Setup()
{
	Enemy::Setup();

	//launchers[0]->position = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
	//launchers[0]->position = GetPosition();


	//TurretSetup();
}

void ShotgunTurret::FireResponse(BasicBullet *b)
{
}

void ShotgunTurret::UpdateBullet(BasicBullet *b)
{
	/*V2d pDir = normalize(sess->GetPlayerPos(0) - b->position);

	b->velocity = pDir * b->launcher->bulletSpeed;*/
}


void ShotgunTurret::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();


	if (frame == actionLength[action] * animFactor[action])
	{
		switch (action)
		{
		case IDLE:
			frame = 0;
			break;
		case ATTACK:
			action = WAIT;
			frame = 0;
			//if (PlayerDist() > 1000)
			//{
			//	action = IDLE;
			//	frame = 0;
			//}
			//else
			//{
			//	frame = 0;
			//	//frame = 0;
			//}
			break;
			
			break;
		case WAIT:
			if (PlayerDist() > 1000)
			{
				action = IDLE;
				frame = 0;
			}
			else
			{
				action = ATTACK;
				frame = 0;
				//frame = 0;
			}
			break;
		}
	}

	switch (action)
	{
	case IDLE:
	{
		if (PlayerDist() < DEFAULT_DETECT_RADIUS )//length(playerPos - position) < 1000)
		{
			action = ATTACK;
			frame = 0;
		}
		break;
	}
	case ATTACK:
	{
		
		
		break;
	}
	}

	switch (action)
	{
	case ATTACK:
	{
		if ((frame == 4 * animFactor[ATTACK] && slowCounter == 1))
			//|| (frame == 1 * animFactor[ATTACK] && slowCounter == 1))
		{
			launchers[0]->facingDir = PlayerDir();
			launchers[0]->Fire();
		}
		break;
	}
	}

}

void ShotgunTurret::UpdateSprite()
{
	int f = 0;
	switch (action)
	{
	case IDLE:
		f = frame / animFactor[IDLE];
		break;
	case ATTACK:
		f = ((frame / animFactor[ATTACK]) % 14) + 9;
		break;
	case WAIT:
		f = 0;
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(f));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

int ShotgunTurret::GetNumStoredBytes()
{
	return sizeof(MyData) + GetNumStoredLauncherBytes();
}

void ShotgunTurret::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	StoreBytesForLaunchers(bytes);
	bytes += GetNumStoredLauncherBytes();
}

void ShotgunTurret::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	SetLaunchersFromBytes(bytes);
	bytes += GetNumStoredLauncherBytes();
}

