#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SKYDIVETOFALL_Start()
{
}

void Actor::SKYDIVETOFALL_End()
{
}

void Actor::SKYDIVETOFALL_Change()
{
}

void Actor::SKYDIVETOFALL_Update()
{
}

void Actor::SKYDIVETOFALL_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::SKYDIVETOFALL_TransitionToAction(int a)
{

}

void Actor::SKYDIVETOFALL_TimeIndFrameInc()
{

}

void Actor::SKYDIVETOFALL_TimeDepFrameInc()
{

}

int Actor::SKYDIVETOFALL_GetActionLength()
{
	return 10 * 4;
}

Tileset * Actor::SKYDIVETOFALL_GetTileset()
{
	return GetActionTileset("intro_0_160x80.png");
}