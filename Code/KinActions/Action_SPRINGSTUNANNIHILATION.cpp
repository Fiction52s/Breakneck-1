#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNANNIHILATION_Start()
{
}

void Actor::SPRINGSTUNANNIHILATION_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNANNIHILATION_Change()
{
	if (AttackButtonPressed() )
	{
		SetAction(SPRINGSTUNANNIHILATIONATTACK);
	}
	else if (GlideAction())
	{
		springStunFrames = 0;
	}
	else
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
}

void Actor::SPRINGSTUNANNIHILATION_Update()
{
	/*if (currInput.LLeft())
	{
		if (facingRight)
		{
			springVel.x = -springVel.x;
		}
		facingRight = false;
	}
	else if (currInput.LRight())
	{
		if (!facingRight)
		{
			springVel.x = -springVel.x;
		}
		facingRight = true;
	}*/


	V2d targetDir = currInput.GetLeft8Dir();

	double turnFactor;

	double len = length(springVel);
	springVel += targetDir * len * .2;
	springVel = normalize(springVel) * len;

	velocity = springVel + springExtra;
}

void Actor::SPRINGSTUNANNIHILATION_UpdateSprite()
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

void Actor::SPRINGSTUNANNIHILATION_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNANNIHILATION_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNANNIHILATION_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNANNIHILATION_GetActionLength()
{
	return 8;
}

Tileset * Actor::SPRINGSTUNANNIHILATION_GetTileset()
{
	return SPRINGSTUN_GetTileset();
}