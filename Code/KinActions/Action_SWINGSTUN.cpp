#include "Actor.h"
#include "Enemy_SwingLauncher.h"

using namespace sf;
using namespace std;

void Actor::SWINGSTUN_Start()
{
}

void Actor::SWINGSTUN_End()
{
	frame = 0;
}

void Actor::SWINGSTUN_Change()
{
	if (!GlideAction())
	{
		if (springStunFrames == 0)
		{
			SetAction(JUMP);
			frame = 1;
			if (velocity.x < 0)
			{
				facingRight = false;
			}
			else if (velocity.x > 0)
			{
				facingRight = true;
			}
		}
	}
	else
	{
		springStunFrames = 0;
	}
}

void Actor::SWINGSTUN_Update()
{
	double rad = oldSwingLauncher->swingRadius;
	double speed = oldSwingLauncher->speed;

	V2d anchor = oldSwingLauncher->anchor;
	V2d future = position + normalize(springVel) * speed;//velocity;

														 //V2d seg = wirePoint - rwPos;
														 //double segLength = length(seg);
	V2d diff = anchor - future;

	if (length(diff) > rad)
	{
		future += normalize(diff) * (length(diff) - rad);
		springVel = future - position;
	}

	velocity = springVel; //* speed;
}

void Actor::SWINGSTUN_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
}

void Actor::SWINGSTUN_TransitionToAction(int a)
{

}

void Actor::SWINGSTUN_TimeIndFrameInc()
{

}

void Actor::SWINGSTUN_TimeDepFrameInc()
{

}

int Actor::SWINGSTUN_GetActionLength()
{
	return 1;
}

Tileset * Actor::SWINGSTUN_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}