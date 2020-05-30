#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_KINTHROWN_Start()
{
}

void Actor::SEQ_KINTHROWN_End()
{
	frame = actionLength[SEQ_KINTHROWN] = 1;
}

void Actor::SEQ_KINTHROWN_Change()
{
}

void Actor::SEQ_KINTHROWN_Update()
{
}

void Actor::SEQ_KINTHROWN_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}