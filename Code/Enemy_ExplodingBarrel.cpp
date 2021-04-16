#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_ExplodingBarrel.h"
#include "Eye.h"

using namespace std;
using namespace sf;


#define COLOR_TEAL Color( 0, 0xee, 0xff )
#define COLOR_BLUE Color( 0, 0x66, 0xcc )

ExplodingBarrel::ExplodingBarrel(ActorParams *ap)
	:Enemy(EnemyType::EN_EXPLODINGBARREL, ap)//, false, 1, false)
{
	SetNumActions(S_Count);
	SetEditorActions(S_CHARGE, S_CHARGE, 0);

	actionLength[S_IDLE] = 15;
	actionLength[S_TINYCHARGE] = 3;
	actionLength[S_CHARGE] = 12;
	actionLength[S_EXPLODE] = 20;

	animFactor[S_IDLE] = 1;
	animFactor[S_CHARGE] = 5;
	animFactor[S_TINYCHARGE] = 5;
	animFactor[S_EXPLODE] = 1;


	SetLevel(ap->GetLevel());

	SetCurrPosInfo(startPosInfo);

	facingRight = true;

	ts = sess->GetSizedTileset("Enemies/W3/barrel_128x128.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	SetOffGroundHeight(ts->tileHeight / 2.0 - 20);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHurtBodySetup(48);
	BasicCircleHitBodySetup(48);
	hitBody.hitboxInfo = hitboxInfo;

	comboObj = new ComboObject(this);

	double explosionRadius = 300;

	//explosion.ResetFrames();
	explosion.BasicCircleSetup(explosionRadius * scale, 0, V2d());
	explosion.hitboxInfo = hitboxInfo;

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

	comboObj->enemyHitboxFrame = 0;

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
	comboObj->enemyHitboxFrame = 0;

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
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
		ConfirmHitNoKill();
		action = S_EXPLODE;
		frame = 0;
		SetHitboxes(&explosion, 0);
		HurtboxesOff();
		

		sess->PlayerAddActiveComboObj(comboObj, GetReceivedHitPlayerIndex());
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
		case S_TINYCHARGE:
		case S_CHARGE:
			action = S_IDLE;
			break;
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
		
		
	}
	sprite.setTextureRect(ts->GetSubRect(tile));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setRotation(currPosInfo.GetGroundAngleDegrees());
}

void ExplodingBarrel::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);

	if (action == S_EXPLODE)
	{
		target->draw(testCircle);
	}
}