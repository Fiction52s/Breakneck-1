#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::RAILDASH_Start()
{
}

void Actor::RAILDASH_End()
{
	SetActionExpr(JUMP);
	frame = 1;
}

void Actor::RAILDASH_Change()
{
	if (!BasicAirAction())
	{
		if (!currInput.B)
		{
			SetAction(JUMP);
			frame = 1;
		}
	}
}

void Actor::RAILDASH_Update()
{
}

void Actor::RAILDASH_UpdateSprite()
{
	sprite->setPosition(position.x, position.y);
}

void Actor::RAILDASH_TransitionToAction(int a)
{

}

void Actor::RAILDASH_TimeIndFrameInc()
{

}

void Actor::RAILDASH_TimeDepFrameInc()
{

}

int Actor::RAILDASH_GetActionLength()
{
	return 1;
}

Tileset * Actor::RAILDASH_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}