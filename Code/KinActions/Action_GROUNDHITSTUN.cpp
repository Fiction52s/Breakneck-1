#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::GROUNDHITSTUN_Start()
{
	SetExpr(KinMask::Expr::Expr_HURT);
}

void Actor::GROUNDHITSTUN_End()
{
	if (stunBufferedAttack == Action::Count)
	{
		if (currInput.rightShoulder && !prevInput.rightShoulder)
		{
			stunBufferedAttack = STANDN;
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
}

void Actor::GROUNDHITSTUN_Change()
{
	
}

void Actor::GROUNDHITSTUN_Update()
{
	hitstunFrames--;
	int slowDown = 1;
	if (groundSpeed > 0)
	{
		groundSpeed -= slowDown;
		if (groundSpeed < 0)
		{
			groundSpeed = 0;
		}
	}
	else if (groundSpeed < 0)
	{
		groundSpeed += slowDown;
		if (groundSpeed > 0)
		{
			groundSpeed = 0;
		}
	}
}

void Actor::GROUNDHITSTUN_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(1, r);


	SetGroundedSpriteTransform();
}

void Actor::GROUNDHITSTUN_TransitionToAction(int a)
{
	if (desperationMode)
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

void Actor::GROUNDHITSTUN_TimeIndFrameInc()
{

}

void Actor::GROUNDHITSTUN_TimeDepFrameInc()
{

}

int Actor::GROUNDHITSTUN_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDHITSTUN_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}