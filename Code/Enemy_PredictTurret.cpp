#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_PredictTurret.h"
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



PredictTurret::PredictTurret(ActorParams *ap)
	:Enemy(EnemyType::EN_PREDICTTURRET, ap)
{
	SetNumActions(Count);
	SetEditorActions(ATTACK, 0, 0);

	SetLevel(ap->GetLevel());

	framesWait = 60;
	bulletSpeed = 10;
	animationFactor = 3;
	assert(framesWait > 13 * animationFactor);

	ts = GetSizedTileset("Enemies/W2/curveturret_144x96.png");

	futureFrames = 30;

	double width = ts->tileWidth;
	double height = ts->tileHeight;

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
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 15;
	hitboxInfo->knockback = 10;
	hitboxInfo->hType = HitboxInfo::MAGENTA;

	BasicCircleHurtBodySetup(32);
	BasicCircleHitBodySetup(32);
	hitBody.hitboxInfo = hitboxInfo;

	bulletSpeed = 10;

	SetNumLaunchers(1);
	launchers[0] = new Launcher(this,
		BasicBullet::PREDICT, 32, 1, GetPosition(), V2d(0, -1), 0, 180, false);
	launchers[0]->SetBulletSpeed(bulletSpeed);
	launchers[0]->hitboxInfo->damage = 60;

	cutObject->SetTileset(ts);
	cutObject->SetSubRectFront(12);
	cutObject->SetSubRectBack(11);
	cutObject->SetScale(scale);
	cutObject->rotateAngle = sprite.getRotation();

	UpdateOnPlacement(ap);

	Color circleColor = Color::Red;
	circleColor.a = 100;
	testCircle.setFillColor(circleColor);
	testCircle.setRadius(20);
	testCircle.setOrigin(testCircle.getLocalBounds().width / 2,
		testCircle.getLocalBounds().height / 2);


	ResetEnemy();
}

void PredictTurret::ResetEnemy()
{
	action = WAIT;
	frame = 0;
	DefaultHurtboxesOn();
	DefaultHitboxesOn();

	if (startPosInfo.ground != NULL)
	{
		launchers[0]->position = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
		
		//launchers[0]->facingDir = startPosInfo.GetEdge()->Normal();
	}
	
	//currShield = shield;
	//shield->Reset();

	UpdateHitboxes();
	UpdateSprite();
}

void PredictTurret::SetLevel(int lev)
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

void PredictTurret::Setup()
{
	Enemy::Setup();

	//launchers[0]->position = startPosInfo.GetEdge()->GetRaisedPosition(startPosInfo.GetQuant(), 80.0 * (double)scale);
	//launchers[0]->position = GetPosition();


	//TurretSetup();
}

void PredictTurret::FireResponse(BasicBullet *b)
{
}

void PredictTurret::UpdateBullet(BasicBullet *b)
{
}

void PredictTurret::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);
	V2d position = GetPosition();
	switch (action)
	{
	case WAIT:
	{
		if (length(playerPos - position) < 1000)
		{
			action = ATTACK;
			frame = 0;
		}
		break;
	}
	case ATTACK:
	{
		if (frame == 13 * animationFactor)
		{
			frame = 0;
			if (length(playerPos - position) >= 500)
			{
				action = WAIT;
				frame = 0;
			}
		}
		else if (frame == 4 * animationFactor && slowCounter == 1)
		{
			V2d launchPos = launchers[0]->position;
			V2d futurePos = sess->GetFuturePlayerPos(futureFrames);
			V2d futureDir = normalize(futurePos - launchPos);

			double futureDist = length(futurePos - launchPos);

			launchers[0]->facingDir = futureDir;

			launchers[0]->SetBulletSpeed(futureDist / futureFrames);

			launchers[0]->Fire();

			testCircle.setPosition(Vector2f(futurePos));
		}
		break;
	}
	}

}

void PredictTurret::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	target->draw(testCircle);
}


void PredictTurret::UpdateSprite()
{
	if (action == WAIT)
	{
		sprite.setTextureRect(ts->GetSubRect(0));
	}
	else
	{
		if (frame / animationFactor > 12)
		{
			sprite.setTextureRect(ts->GetSubRect(0));
		}
		else
		{
			sprite.setTextureRect(ts->GetSubRect(frame / animationFactor));//frame / animationFactor ) );
		}
	}

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

void PredictTurret::DebugDraw(sf::RenderTarget *target)
{
	Enemy::DebugDraw(target);
}

void PredictTurret::UpdatePreFrameCalculations()
{
	if (action == ATTACK && (frame == 4 * animationFactor && slowCounter == 1))
	{
		sess->PlayerMustSimulateAtLeast(GetNumSimulationFramesRequired(), 0);
	}
}

int PredictTurret::GetNumSimulationFramesRequired()
{
	return futureFrames + 2;
}

int PredictTurret::GetNumStoredBytes()
{
	return sizeof(MyData) + launchers[0]->GetNumStoredBytes();
}

void PredictTurret::StoreBytes(unsigned char *bytes)
{
	StoreBasicEnemyData(data);
	memcpy(bytes, &data, sizeof(MyData));
	bytes += sizeof(MyData);

	launchers[0]->StoreBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}

void PredictTurret::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));
	SetBasicEnemyData(data);
	bytes += sizeof(MyData);

	launchers[0]->SetFromBytes(bytes);
	bytes += launchers[0]->GetNumStoredBytes();
}