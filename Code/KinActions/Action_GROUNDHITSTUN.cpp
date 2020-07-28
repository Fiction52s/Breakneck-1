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
}

void Actor::GROUNDHITSTUN_Change()
{
	if (hitstunFrames == 0)
	{
		BasicGroundAction(currNormal);
	}
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
	if (kinMode == K_DESPERATION)
	{
		SetExpr(KinMask::Expr::Expr_DESP);
	}
	else
	{
		SetExpr(KinMask::Expr::Expr_NEUTRAL);
	}
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