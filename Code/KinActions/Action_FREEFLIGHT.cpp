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
	if (freeFlightFrames == 0)
	{
		SetAction(JUMP);
		frame = 1;
		gravModifyFrames = 0;
	}
	else
	{
		if (BasicAirAction())
		{
		}
	}
}

void Actor::FREEFLIGHT_Update()
{
	FreeFlightMovement();
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

const char * Actor::FREEFLIGHT_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}