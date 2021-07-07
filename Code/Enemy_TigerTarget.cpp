#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_TigerTarget.h"
#include "Actor.h"

using namespace std;
using namespace sf;

TigerTarget::TigerTarget(ActorParams *ap)
	:Enemy(EnemyType::EN_TIGERTARGET, ap)
{
	SetLevel(ap->GetLevel());

	SetNumActions(A_Count);
	SetEditorActions(NEUTRAL, NEUTRAL, 0);

	actionLength[NEUTRAL] = 2;

	ts = GetSizedTileset("Bosses/Coyote/babyscorpion_64x64.png");
	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 18;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	BasicCircleHitBodySetup(16);
	BasicCircleHurtBodySetup(16);

	hitBody.hitboxInfo = hitboxInfo;

	//cutObject->Setup(ts, 0, 0, scale);

	ResetEnemy();
}

void TigerTarget::SetLevel(int lev)
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

//void TigerTarget::ProcessHit()
//{
//	if (!dead && ReceivedHit() && numHealth > 0)
//	{
//		sess->PlayerConfirmEnemyNoKill(this, GetReceivedHitPlayerIndex());
//		ConfirmHitNoKill();
//
//		if (receivedHit->hType == HitboxInfo::COMBO)
//		{
//			comboHitEnemy->ComboKill(this);
//		}
//
//		action = DISSIPATE;
//		frame = 0;
//		HitboxesOff();
//		HurtboxesOff();
//	}
//}

void TigerTarget::ResetEnemy()
{
	action = NEUTRAL;
	frame = 0;

	DefaultHitboxesOn();
	DefaultHurtboxesOn();

	UpdateHitboxes();

	UpdateSprite();
}

void TigerTarget::ActionEnded()
{
	V2d playerPos = sess->GetPlayerPos();

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
		switch (action)
		{
		case NEUTRAL:
			break;
		}
	}
}

void TigerTarget::ProcessState()
{
	ActionEnded();

	double dist = PlayerDist();
	V2d dir = PlayerDir();

	switch (action)
	{
	case NEUTRAL:
		break;
	}

	switch (action)
	{
	case NEUTRAL:
		break;
		break;
	}
}

void TigerTarget::UpdateEnemyPhysics()
{
	/*V2d movementVec = velocity;
	movementVec /= slowMultiple * (double)numPhysSteps;

	currPosInfo.position += movementVec;*/
}

void TigerTarget::UpdateSprite()
{
	int trueFrame;
	switch (action)
	{
	case NEUTRAL:
		//sprite.setColor(Color::White);
		break;

		break;
	}

	ts->SetSubRect(sprite, 0, !facingRight);
	sprite.setOrigin(sprite.getLocalBounds().width / 2,
		sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void TigerTarget::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}