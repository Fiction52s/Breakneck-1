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