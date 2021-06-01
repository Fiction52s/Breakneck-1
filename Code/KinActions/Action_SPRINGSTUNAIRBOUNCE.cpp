#include "Actor.h"
#include "Session.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNAIRBOUNCE_Start()
{
	airBounceLimit = 3;
}

void Actor::SPRINGSTUNAIRBOUNCE_End()
{
	frame = GetActionLength(SPRINGSTUNAIRBOUNCE) - 1;
}

void Actor::SPRINGSTUNAIRBOUNCE_Change()
{
	CheckBounceFlame();

	if (airBounceCounter < airBounceLimit)
	{
		if (JumpButtonPressed())
		{
			SetAction(SPRINGSTUNAIRBOUNCEPAUSE);

			V2d dir8 = currInput.GetLeft8Dir();

			if (dir8.x != 0 || dir8.y != 0)
			{
				springVel = dir8 * length(springVel);
			}
			return;
		}
	}
	else
	{
		if (TryDoubleJump())
		{
			springStunFrames = 0;
			return;
		}
	}

	if (TryAirDash()) 
	{
		springStunFrames = 0;
		return;
	}

	if (TryWallJump())
	{
		springStunFrames = 0;
		return;
	}

	if (AirAttack())
	{
		if (velocity.x < 0)
		{
			facingRight = false;
		}
		else if (velocity.x > 0)
		{
			facingRight = true;
		}
		
		
		springStunFrames = 0;
		return;
		
	}

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

void Actor::SPRINGSTUNAIRBOUNCE_Update()
{

	velocity = springVel + springExtra;

	if (velocity.x < 0)
	{
		facingRight = false;
	}
	else if (velocity.x > 0)
	{
		facingRight = true;
	}

	ActivateLauncherEffect(6);
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