#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_WireTarget.h"
//#include "Eye.h"
#include "KeyMarker.h"

using namespace std;
using namespace sf;

WireTarget::WireTarget(GameSession *owner, bool p_hasMonitor, Vector2i pos, int p_level )
	:Enemy(owner, EnemyType::EN_WIRETARGET, p_hasMonitor, 1, false)
{
	level = p_level;

	switch (level)
	{
	case 1:
		scale = 2.0;
		break;
	case 2:
		scale = 3.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 4.0;
		maxHealth += 5;
		break;
	}

	position.x = pos.x;
	position.y = pos.y;
	origPos = position;
	spawnRect = sf::Rect<double>(pos.x - 16, pos.y - 16, 16 * 2, 16 * 2);

	ts = owner->GetTileset("Enemies/Comboer_128x128.png", 128, 128);
	sprite.setTexture(*ts->texture);

	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos.x, pos.y);

	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	SetupBodies(1, 1);
	AddBasicHitCircle(48);
	AddBasicHurtCircle(48);
	hitBody->hitboxInfo = hitboxInfo;

	SetHitboxes(hitBody, 0);

	actionLength[S_FLOAT] = 18;
	actionLength[S_DESTROY] = 10;

	animFactor[S_FLOAT] = 2;
	animFactor[S_DESTROY] = 1;

	ResetEnemy();
}

WireTarget::~WireTarget()
{
}

bool WireTarget::CanBeHitByComboer()
{
	return false;
}

bool WireTarget::CanBeHitByWireTip(bool red)
{
	return true;
}

void WireTarget::ResetEnemy()
{
	sprite.setTextureRect(ts->GetSubRect(0));
	sprite.setRotation(0);

	SetHitboxes(hitBody, 0);
	SetHurtboxes(hurtBody, 0);

	dead = false;
	action = S_FLOAT;
	frame = 0;
	receivedHit = NULL;
	position = origPos;
	UpdateHitboxes();

	sprite.setColor(Color::Magenta);

	UpdateSprite();
}



void WireTarget::ProcessHit()
{
	if (!dead && ReceivedHit() && numHealth > 0)
	{
		if (receivedHit->hType == HitboxInfo::WIREHITRED)
		{
			numHealth -= 1;
		}
		else if (receivedHit->hType == HitboxInfo::WIREHITBLUE)
		{
			numHealth -= 1;
		}

		if (numHealth <= 0)
		{
			owner->PlayerConfirmEnemyKill(this);
			ConfirmKill();
			action = S_DESTROY;
			frame = 0;
			sprite.setColor(Color::Black);
		}
		else
		{
			owner->PlayerConfirmEnemyNoKill(this);
			ConfirmHitNoKill();
		}
	}
}

void WireTarget::ProcessState()
{
	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;

		switch (action)
		{
		case S_DESTROY:
			SetHitboxes(hitBody, 0);
			SetHurtboxes(hurtBody, 0);
			numHealth = 0;
			dead = true;
			break;
		}
	}
}

bool WireTarget::CanBeAnchoredByWire(bool red)
{
	return false;
}

void WireTarget::HandleNoHealth()
{

}

void WireTarget::UpdateSprite()
{
	sprite.setPosition(position.x, position.y);

	int tile = 0;
	switch (action)
	{
	case S_FLOAT:
		tile = 0;
	case S_DESTROY:
		tile = 0;
		break;
	}

	sprite.setTextureRect(ts->GetSubRect(tile));
}

void WireTarget::EnemyDraw(sf::RenderTarget *target)
{
	DrawSpriteIfExists(target, sprite);
}