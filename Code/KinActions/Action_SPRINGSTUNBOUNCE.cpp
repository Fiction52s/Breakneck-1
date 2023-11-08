#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNBOUNCE_Start()
{
}

void Actor::SPRINGSTUNBOUNCE_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNBOUNCE_Change()
{
	//doesnt use glide action because you shouldn't be able to walljump
	CheckBounceFlame();

	if (TryDoubleJump())
	{
		springStunFrames = 0;
		return;
	}

	if (TryAirDash())
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

void Actor::SPRINGSTUNBOUNCE_Update()
{
	double bounceTurnFactor = .012;

	glideTurnFactor = 0;


	if (facingRight)
	{
		RotateCCW(springVel, glideTurnFactor);
	}
	else
	{
		RotateCCW(springVel, -glideTurnFactor);
	}

	velocity = springVel + springExtra;

	ActivateLauncherEffect(4);
}

void Actor::SPRINGSTUNBOUNCE_UpdateSprite()
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

void Actor::SPRINGSTUNBOUNCE_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNBOUNCE_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNBOUNCE_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNBOUNCE_GetActionLength()
{
	return 8;
}

const char * Actor::SPRINGSTUNBOUNCE_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}