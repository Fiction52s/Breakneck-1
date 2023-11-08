#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::WALLTECH_Start()
{
	if (currInput.LUp())
	{
		if (velocity.y > 0)
		{
			velocity.y = -10;
		}
	}
	else if (currInput.LDown())
	{
		if (velocity.y < 0)
		{
			velocity.y = 10;
		}
	}
	else
	{
		velocity.y = 0;
	}
}

void Actor::WALLTECH_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::WALLTECH_Change()
{

}

void Actor::WALLTECH_Update()
{
}

void Actor::WALLTECH_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(6, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::WALLTECH_TransitionToAction(int a)
{
}

void Actor::WALLTECH_TimeIndFrameInc()
{

}

void Actor::WALLTECH_TimeDepFrameInc()
{

}

int Actor::WALLTECH_GetActionLength()
{
	return 20;
}

const char * Actor::WALLTECH_GetTilesetName()
{
	return "jump_64x64.png";
}