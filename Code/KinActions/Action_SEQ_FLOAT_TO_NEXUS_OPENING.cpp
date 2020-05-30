#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Start()
{
}

void Actor::SEQ_FLOAT_TO_NEXUS_OPENING_End()
{
	action = SEQ_FADE_INTO_NEXUS;
	frame = 0;
}

void Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Change()
{
}

void Actor::SEQ_FLOAT_TO_NEXUS_OPENING_Update()
{
}

void Actor::SEQ_FLOAT_TO_NEXUS_OPENING_UpdateSprite()
{
	SetSpriteTexture(action);
	SetSpriteTile(frame / 10, facingRight);
	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2 + 24);
	sprite->setPosition(Vector2f(position));
	//sprite->setPosition(owner->goalNodePos.x, owner->goalNodePos.y - 24.f);//- 24.f );
	//sprite->setPosition(Vector2f(position));
	sprite->setRotation(0);
}