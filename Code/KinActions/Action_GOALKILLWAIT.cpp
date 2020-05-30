#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GOALKILLWAIT_Start()
{
}

void Actor::GOALKILLWAIT_End()
{
	frame = 0;
}

void Actor::GOALKILLWAIT_Change()
{
}

void Actor::GOALKILLWAIT_Update()
{
}

void Actor::GOALKILLWAIT_UpdateSprite()
{
	SetSpriteTexture(GOALKILL4);
	SetSpriteTile(7, facingRight);
	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2 + 24);
	sprite->setPosition(Vector2f(position));
	//sprite->setPosition( owner->goalNodePos.x, owner->goalNodePos.y - 24.f );
	sprite->setRotation(0);
}