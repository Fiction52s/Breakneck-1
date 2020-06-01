#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SKYDIVE_Start()
{
}

void Actor::SKYDIVE_End()
{
}

void Actor::SKYDIVE_Change()
{
}

void Actor::SKYDIVE_Update()
{
}

void Actor::SKYDIVE_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::SKYDIVE_TransitionToAction(int a)
{

}

void Actor::SKYDIVE_TimeIndFrameInc()
{

}

void Actor::SKYDIVE_TimeDepFrameInc()
{

}

int Actor::SKYDIVE_GetActionLength()
{
	return 9 * 2;
}

Tileset * Actor::SKYDIVE_GetTileset()
{
	return GetActionTileset("walljump_64x64.png");
}