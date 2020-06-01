#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::ENTERNEXUS1_Start()
{
}

void Actor::ENTERNEXUS1_End()
{
}

void Actor::ENTERNEXUS1_Change()
{
}

void Actor::ENTERNEXUS1_Update()
{
}

void Actor::ENTERNEXUS1_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(frame / 4, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::ENTERNEXUS1_TransitionToAction(int a)
{

}

void Actor::ENTERNEXUS1_TimeIndFrameInc()
{

}

void Actor::ENTERNEXUS1_TimeDepFrameInc()
{

}

int Actor::ENTERNEXUS1_GetActionLength()
{
	return 1;
}

Tileset * Actor::ENTERNEXUS1_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}