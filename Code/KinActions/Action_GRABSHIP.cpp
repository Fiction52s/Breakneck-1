#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GRABSHIP_Start()
{
}

void Actor::GRABSHIP_End()
{
}

void Actor::GRABSHIP_Change()
{
}

void Actor::GRABSHIP_Update()
{
}

void Actor::GRABSHIP_UpdateSprite()
{
	if (frame / 5 < 8)
	{
		SetSpriteTexture(action);

		//bool r = (facingRight && !reversed ) || (!facingRight && reversed );
		SetSpriteTile(1 + frame / 5, true);

		sprite->setOrigin(sprite->getLocalBounds().width / 2,
			sprite->getLocalBounds().height / 2);
	}
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}