#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::EXIT_Start()
{
}

void Actor::EXIT_End()
{
	SetAction(EXITWAIT);
	frame = 0;
}

void Actor::EXIT_Change()
{
}

void Actor::EXIT_Update()
{
}

void Actor::EXIT_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(frame / 2, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2,
		sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);//position.x, position.y );
	sprite->setRotation(0);
}

void Actor::EXIT_TransitionToAction(int a)
{

}

void Actor::EXIT_TimeIndFrameInc()
{

}

void Actor::EXIT_TimeDepFrameInc()
{

}

int Actor::EXIT_GetActionLength()
{
	return 29 * 2;
}

Tileset * Actor::EXIT_GetTileset()
{
	return GetActionTileset("exit_64x128.png");
}