#include "Actor.h"
#include "Enemy.h"

using namespace sf;
using namespace std;

void Actor::SPRINGSTUNHOMINGATTACK_Start()
{
	
}

void Actor::SPRINGSTUNHOMINGATTACK_End()
{
	frame = 0;
}

void Actor::SPRINGSTUNHOMINGATTACK_Change()
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

	if ( springStunFrames > 0 && (!AttackButtonHeld() || !TryHomingMovement()))
	{
		SetAction(SPRINGSTUNHOMING);
	}
}

void Actor::SPRINGSTUNHOMINGATTACK_Update()
{
	//if (AttackButtonHeld())
	{
		Enemy *foundEnemy = NULL;
		int foundIndex;

		if (GetClosestEnemyPos(position, 2000, foundEnemy, foundIndex))
		{
			V2d eDir = normalize(foundEnemy->GetPosition() - position);
			double len = length(springVel);
			springVel += eDir * 5.0;//len * .1;//len * .5;//10.0;//eDir * 4.0;//eDir * length(springVel);
			springVel = normalize(springVel) * len;
		}

		SetCurrHitboxes(homingHitboxes, 0);
	}

	velocity = springVel + springExtra;

	ActivateLauncherEffect(14);
}

void Actor::SPRINGSTUNHOMINGATTACK_UpdateSprite()
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

void Actor::SPRINGSTUNHOMINGATTACK_TransitionToAction(int a)
{

}

void Actor::SPRINGSTUNHOMINGATTACK_TimeIndFrameInc()
{

}

void Actor::SPRINGSTUNHOMINGATTACK_TimeDepFrameInc()
{

}

int Actor::SPRINGSTUNHOMINGATTACK_GetActionLength()
{
	return 8;
}

const char * Actor::SPRINGSTUNHOMINGATTACK_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}