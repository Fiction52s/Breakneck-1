#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SKYDIVE_Start()
{
}

void Actor::SKYDIVE_End()
{
}

void Actor::SKYDIVE_Change()
{
}

void Actor::SKYDIVE_Update()
{
}

void Actor::SKYDIVE_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}