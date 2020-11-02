#include "Enemy.h"
#include "GameSession.h"
#include <iostream>
#include "VectorMath.h"
#include <assert.h>
#include "Enemy_KeyFly.h"

using namespace std;
using namespace sf;

KeyFly::KeyFly(ActorParams *ap)
	:Enemy(EnemyType::EN_KEYFLY, ap)
{
	SetNumActions(S_Count);
	SetEditorActions(S_NEUTRAL, S_NEUTRAL, 0);

	SetLevel(ap->GetLevel());

	ts = sess->GetSizedTileset("Enemies/healthfly_64x64.png");

	BasicCircleHurtBodySetup(32);

	sprite.setTexture(*ts->texture);
	sprite.setScale(scale, scale);

	actionLength[S_NEUTRAL] = 5;
	animFactor[S_NEUTRAL] = 5;

	cutObject->Setup(ts, 0, 1, scale, 0, false, false);

	ResetEnemy();
}

void KeyFly::SetLevel(int lev)
{
	level = lev;
	switch (level)
	{
	case 1:
		scale = 2.0;
		break;
	case 2:
		scale = 2.0;
		maxHealth += 2;
		break;
	case 3:
		scale = 4.0;
		maxHealth += 5;
		break;
	}
}

void KeyFly::ResetEnemy()
{
	action = S_NEUTRAL;
	frame = 0;

	SetHurtboxes(&hurtBody, 0);

	UpdateHitboxes();
	UpdateSprite();
}



void KeyFly::ProcessState()
{
	V2d playerPos = sess->GetPlayerPos(0);

	if (frame == actionLength[action] * animFactor[action])
	{
		frame = 0;
	}
}

void KeyFly::UpdateSprite()
{
	sprite.setTextureRect(ts->GetSubRect(frame / animFactor[S_NEUTRAL]));

	sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
	sprite.setPosition(GetPositionF());
}

void KeyFly::EnemyDraw(sf::RenderTarget *target)
{
	DrawSprite(target, sprite);
}