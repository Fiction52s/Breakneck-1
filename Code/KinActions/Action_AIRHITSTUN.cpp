#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRHITSTUN_Start()
{
	SetExpr(KinMask::Expr::Expr_HURT);
}

void Actor::AIRHITSTUN_End()
{

}

void Actor::AIRHITSTUN_Change()
{
	if (stunBufferedAttack == Action::Count)
	{
		if (currInput.rightShoulder && !prevInput.rightShoulder)
		{
			if (currInput.LUp())
			{
				stunBufferedAttack = UAIR;
			}
			else if (currInput.LDown())
			{
				stunBufferedAttack = DAIR;
			}
			else
			{
				stunBufferedAttack = FAIR;
			}
		}
	}

	if (!stunBufferedJump && currInput.A && !prevInput.A)
	{
		stunBufferedJump = true;
	}

	if (!stunBufferedDash && currInput.B && !prevInput.B)
	{
		stunBufferedDash = true;
	}


	frame = 0;
	if (hitstunFrames <= setHitstunFrames / 2)
	{
		//AirAttack();
	}
}

void Actor::AIRHITSTUN_Update()
{
	hitstunFrames--;
}

void Actor::AIRHITSTUN_UpdateSprite()
{
	if (frame == 0)
	{
		//playerHitSound.stop();
		//playerHitSound.play();
	}

	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::AIRHITSTUN_TransitionToAction(int a)
{
	if (kinMode == K_DESPERATION)
	{
		SetExpr(KinMask::Expr::Expr_DESP);
	}
	else
	{
		SetExpr(KinMask::Expr::Expr_NEUTRAL);
	}
	
	stunBufferedJump = false;
	stunBufferedDash = false;
	stunBufferedAttack = Action::Count;
}

void Actor::AIRHITSTUN_TimeIndFrameInc()
{

}

void Actor::AIRHITSTUN_TimeDepFrameInc()
{

}

int Actor::AIRHITSTUN_GetActionLength()
{
	return 1;
}

Tileset * Actor::AIRHITSTUN_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}