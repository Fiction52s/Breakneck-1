#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNGRINDFLY_Start()
{
}

void Actor::SPRINGSTUNGRINDFLY_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNGRINDFLY_Change()
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

	/*if (springStunFrames == 0)
	{
	SetAction(JUMP);
	frame = 1;
	}*/
}

void Actor::SPRINGSTUNGRINDFLY_Update()
{
	velocity = springVel + springExtra;
}

void Actor::SPRINGSTUNGRINDFLY_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	V2d sVel = springVel + springExtra;
	if (facingRight)
	{
		double a = GetVectorAngleCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(a);
	}
	else
	{
		double a = GetVectorAngleCCW(normalize(sVel)) * 180 / PI;
		sprite->setRotation(-a + 180);
	}

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::SPRINGSTUNGRINDFLY_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNGRINDFLY_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNGRINDFLY_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNGRINDFLY_GetActionLength()
{
	return 8;
}

Tileset * Actor::SPRINGSTUNGRINDFLY_GetTileset()
{
	return GetActionTileset("launch_96x64.png");
}