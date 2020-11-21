#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::FREEFLIGHT_Start()
{
}

void Actor::FREEFLIGHT_End()
{
	frame = 0;
}

void Actor::FREEFLIGHT_Change()
{
	if (springStunFrames == 0)
	{
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::FREEFLIGHT_Update()
{
	double driftFactor = 1.0;
	double maxAccelSpeed = 15;

	if (currInput.LUp())
	{
		if (velocity.y > -maxAccelSpeed)
		{
			velocity.y -= driftFactor;
		}
		
	}
	if (currInput.LDown())
	{
		if (velocity.y < maxAccelSpeed)
		{
			velocity.y += driftFactor;
		}
	}
	if (currInput.LLeft())
	{
		if (velocity.x > -maxAccelSpeed)
		{
			velocity.x -= driftFactor;
		}
		
	}
	if (currInput.LRight())
	{
		if (velocity.x < maxAccelSpeed)
		{
			velocity.x += driftFactor;
		}
	}
}

void Actor::FREEFLIGHT_UpdateSprite()
{
	SetSpriteTexture(JUMP);

	SetSpriteTile(1, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::FREEFLIGHT_TransitionToAction(int a)
{

}

void Actor::FREEFLIGHT_TimeIndFrameInc()
{

}

void Actor::FREEFLIGHT_TimeDepFrameInc()
{

}

int Actor::FREEFLIGHT_GetActionLength()
{
	return 8;
}

Tileset * Actor::FREEFLIGHT_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}