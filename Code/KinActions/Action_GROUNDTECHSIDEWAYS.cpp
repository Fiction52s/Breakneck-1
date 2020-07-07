#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::GROUNDTECHSIDEWAYS_Start()
{
	if (currInput.LLeft())
	{
		if (groundSpeed >= -10)
		{
			groundSpeed = -10;
		}
	}
	else if (currInput.LRight())
	{
		if (groundSpeed <= 10)
		{
			groundSpeed = 10;
		}
	}
	else
	{
		groundSpeed = 0;
	}
}

void Actor::GROUNDTECHSIDEWAYS_End()
{
	SetAction(STAND);
	frame = 0;
}

void Actor::GROUNDTECHSIDEWAYS_Change()
{

}

void Actor::GROUNDTECHSIDEWAYS_Update()
{
}

void Actor::GROUNDTECHSIDEWAYS_UpdateSprite()
{
	SetSpriteTexture(action);

	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	SetSpriteTile(9, r);


	SetGroundedSpriteTransform();
}

void Actor::GROUNDTECHSIDEWAYS_TransitionToAction(int a)
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

void Actor::GROUNDTECHSIDEWAYS_TimeIndFrameInc()
{

}

void Actor::GROUNDTECHSIDEWAYS_TimeDepFrameInc()
{

}

int Actor::GROUNDTECHSIDEWAYS_GetActionLength()
{
	return 20;
}

Tileset * Actor::GROUNDTECHSIDEWAYS_GetTileset()
{
	return GetActionTileset("jump_64x64.png");
}