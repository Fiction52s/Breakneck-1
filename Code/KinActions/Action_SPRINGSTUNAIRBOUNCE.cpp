#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNAIRBOUNCE_Start()
{
}

void Actor::SPRINGSTUNAIRBOUNCE_End()
{
	frame = GetActionLength(SPRINGSTUNAIRBOUNCE) - 1;
}

void Actor::SPRINGSTUNAIRBOUNCE_Change()
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

void Actor::SPRINGSTUNAIRBOUNCE_Update()
{
	V2d inputDir(0, 0);

	if (frame > 10)
	{


		if (currInput.LUp())
		{
			inputDir.y = -1;
		}
		else if (currInput.LDown())
		{
			inputDir.y = 1;
		}

		if (currInput.LRight())
		{
			inputDir.x = 1;
		}
		else if (currInput.LLeft())
		{
			inputDir.x = -1;
		}


		//bool inputSwitch = true;
		if (inputDir.x != 0 || inputDir.y != 0)
		{
			inputDir = normalize(inputDir);
			springVel = inputDir * length(springVel);
		}
	}


	velocity = springVel + springExtra;

	if (velocity.x < 0)
	{
		facingRight = false;
	}
	else if (velocity.x > 0)
	{
		facingRight = true;
	}
}

void Actor::SPRINGSTUNAIRBOUNCE_UpdateSprite()
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

void Actor::SPRINGSTUNAIRBOUNCE_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNAIRBOUNCE_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNAIRBOUNCE_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNAIRBOUNCE_GetActionLength()
{
	return 30;
}

Tileset * Actor::SPRINGSTUNAIRBOUNCE_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}