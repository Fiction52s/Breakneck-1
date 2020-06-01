#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GETSHARD_Start()
{
}

void Actor::GETSHARD_End()
{
	frame = 0;
}

void Actor::GETSHARD_Change()
{
}

void Actor::GETSHARD_Update()
{
	if (frame == 0)
	{
		if (ground != NULL)
		{
			ground = NULL;
			framesInAir = 0;
			hasAirDash = true;
			hasDoubleJump = true;
		}
	}

	velocity = V2d(0, 0);
}

void Actor::GETSHARD_UpdateSprite()
{
	SetSpriteTexture(action);
	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	SetSpriteTile(0, facingRight);
}

void Actor::GETSHARD_TransitionToAction(int a)
{

}

void Actor::GETSHARD_TimeIndFrameInc()
{

}

void Actor::GETSHARD_TimeDepFrameInc()
{

}

int Actor::GETSHARD_GetActionLength()
{
	return 2;
}

Tileset * Actor::GETSHARD_GetTileset()
{
	return DEATH_GetTileset();
}