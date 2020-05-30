#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::INTRO_Start()
{
}

void Actor::INTRO_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::INTRO_Change()
{
}

void Actor::INTRO_Update()
{
}

void Actor::INTRO_UpdateSprite()
{
	if (frame == 0 && slowCounter == 1)
	{
		ActivateEffect(EffectLayer::IN_FRONT, GetTileset("Kin/enter_fx_320x320.png", 320, 320), position, false, 0, 22, 2, true);
	}
	else if (frame / 2 >= 5)
	{
		SetSpriteTexture(action);
		SetSpriteTile((frame - 5) / 2, facingRight);

		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2);
		sprite->setPosition(position.x, position.y);
		sprite->setRotation(0);
	}
}