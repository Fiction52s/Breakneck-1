#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUN_Start()
{
}

void Actor::SPRINGSTUN_End()
{
	frame = 0;
}

void Actor::SPRINGSTUN_Change()
{
	if (springStunFrames == 0)
	{
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::SPRINGSTUN_Update()
{
	velocity = springVel + springExtra;
}

void Actor::SPRINGSTUN_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	V2d sVel = springVel + springExtra;
	if (facingRight)
	{
		double a = GetVectorAngleCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(a);
	}
	else
	{
		double a = GetVectorAngleCCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(-a + 180);
	}

	if (scorpOn)
		SetAerialScorpSprite();
}