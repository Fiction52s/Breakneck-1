#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::RIDESHIP_Start()
{
}

void Actor::RIDESHIP_End()
{
}

void Actor::RIDESHIP_Change()
{
}

void Actor::RIDESHIP_Update()
{
}

void Actor::RIDESHIP_UpdateSprite()
{
	int tFrame = (frame - 90) / 5;

	if (tFrame < 0)
	{
		tFrame = 0;
	}
	else if (tFrame >= 5)
	{
		tFrame = 5;
	}
	else
	{
		tFrame++;
	}

	SetSpriteTexture(action);
	SetSpriteTile(tFrame, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::RIDESHIP_TransitionToAction(int a)
{

}

void Actor::RIDESHIP_TimeIndFrameInc()
{

}

void Actor::RIDESHIP_TimeDepFrameInc()
{

}

int Actor::RIDESHIP_GetActionLength()
{
	return 1;
}

Tileset * Actor::RIDESHIP_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}