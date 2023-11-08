#include "Actor.h"
#include "Enemy.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNHOMING_Start()
{
}

void Actor::SPRINGSTUNHOMING_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNHOMING_Change()
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

	if (HomingAction())
	{
		springStunFrames = 0;
		return;
	}
	else
	{
		
	}

	if (AttackButtonPressed() && TryHomingMovement())
	{
		SetAction(SPRINGSTUNHOMINGATTACK);
	}

	ActivateLauncherEffect(12);
}

void Actor::SPRINGSTUNHOMING_Update()
{
	velocity = springVel + springExtra;
}

void Actor::SPRINGSTUNHOMING_UpdateSprite()
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

void Actor::SPRINGSTUNHOMING_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNHOMING_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNHOMING_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNHOMING_GetActionLength()
{
	return 8;
}

const char * Actor::SPRINGSTUNHOMING_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}