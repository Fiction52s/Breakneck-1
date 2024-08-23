#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::EXIT_RUSH_Start()
{
}

void Actor::EXIT_RUSH_End()
{
	SetAction(EXITWAIT);
	frame = 0;
}

void Actor::EXIT_RUSH_Change()
{
}

void Actor::EXIT_RUSH_Update()
{
}

void Actor::EXIT_RUSH_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y - 16);//position.x, position.y );
	sprite->setRotation(0);
}

void Actor::EXIT_RUSH_TransitionToAction(int a)
{

}

void Actor::EXIT_RUSH_TimeIndFrameInc()
{

}

void Actor::EXIT_RUSH_TimeDepFrameInc()
{

}

int Actor::EXIT_RUSH_GetActionLength()
{
	return 29 * 2;
}

const char * Actor::EXIT_RUSH_GetTilesetName()
{
	return "exit_64x128.png";
}