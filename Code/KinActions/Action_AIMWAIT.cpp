#include "Actor.h"
#include "Enemy_AimLauncher.h"

using namespace sf;
using namespace std;

void Actor::AIMWAIT_Start()
{
}

void Actor::AIMWAIT_End()
{
	frame = 0;
}

void Actor::AIMWAIT_Change()
{
	V2d dir8 = currInput.GetLeft8Dir();

	if (dir8.x != 0 || dir8.y != 0)
	{
		currAimLauncher->SetCurrDir(dir8);
	}
	else
	{
		currAimLauncher->SetCurrDir(currAimLauncher->origDir);
	}


	if (JumpButtonPressed())
	{
		int aimLauncherType = currAimLauncher->aimLauncherType;
		if (aimLauncherType == AimLauncher::TYPE_BOUNCE)
		{
			SetAction(SPRINGSTUNAIM);
			aimLauncherStunFrames = currAimLauncher->stunFrames;
		}
		else if (aimLauncherType == AimLauncher::TYPE_GRIND)
		{
			SetAction(SPRINGSTUNGRINDFLY);
		}
		else if (aimLauncherType == AimLauncher::TYPE_HOMING)
		{
			SetAction(SPRINGSTUNHOMING);
		}
		
		frame = 0;
		currAimLauncher->Launch();

		double s = currAimLauncher->speed;

		springVel = currAimLauncher->currDir * s;

		springExtra = V2d(0, 0);

		springStunFrames = currAimLauncher->stunFrames;

		currAimLauncher = NULL;
	}
	/*if (!GlideAction())
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
	}*/
}

void Actor::AIMWAIT_Update()
{
	velocity = V2d(0, 0);//springVel + springExtra;
}

void Actor::AIMWAIT_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
	/*
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
		SetAerialScorpSprite();*/
}

void Actor::AIMWAIT_TransitionToAction(int a)
{

}

void Actor::AIMWAIT_TimeIndFrameInc()
{

}

void Actor::AIMWAIT_TimeDepFrameInc()
{

}

int Actor::AIMWAIT_GetActionLength()
{
	return 8;
}

Tileset * Actor::AIMWAIT_GetTileset()
{
	return GetActionTileset("launch_96x64.png");
}