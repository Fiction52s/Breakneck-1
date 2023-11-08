#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRPARRY_Start()
{
}

void Actor::AIRPARRY_End()
{
	if (PowerButtonHeld())
	{
		SetAirBlockAction();
		frame = 0;
	}
	else
	{
		SetAction(JUMP);
		frame = 1;
	}
}

void Actor::AIRPARRY_Change()
{
	if (frame > 0)
	{
		SetAirBlockAction();
	}
}

void Actor::AIRPARRY_Update()
{
	//CheckHoldJump();
}

void Actor::AIRPARRY_UpdateSprite()
{
	SetSpriteTexture(action);
	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::AIRPARRY_TransitionToAction(int a)
{
}

void Actor::AIRPARRY_TimeIndFrameInc()
{

}

void Actor::AIRPARRY_TimeDepFrameInc()
{

}

int Actor::AIRPARRY_GetActionLength()
{
	return 15;
}

const char * Actor::AIRPARRY_GetTilesetName()
{
	return "parry_64x64.png";
}