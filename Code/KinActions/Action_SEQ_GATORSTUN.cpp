#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_GATORSTUN_Start()
{
}

void Actor::SEQ_GATORSTUN_End()
{
}

void Actor::SEQ_GATORSTUN_Change()
{
}

void Actor::SEQ_GATORSTUN_Update()
{
	frame = 0;
}

void Actor::SEQ_GATORSTUN_UpdateSprite()
{
	int f = (frame / 4) % 2;

	SetSpriteTexture(action);
	SetSpriteTile(f+1, facingRight);
	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(Vector2f(position));
	sprite->setRotation(0);
}

void Actor::SEQ_GATORSTUN_TransitionToAction(int a)
{

}

void Actor::SEQ_GATORSTUN_TimeIndFrameInc()
{

}

void Actor::SEQ_GATORSTUN_TimeDepFrameInc()
{

}

int Actor::SEQ_GATORSTUN_GetActionLength()
{
	return 10;
}

const char * Actor::SEQ_GATORSTUN_GetTilesetName()
{
	return "hurt_64x64.png";
}