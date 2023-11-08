#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::RAILBOUNCE_Start()
{
	//SetExpr(KinMask::Expr::Expr_HURT);
}

void Actor::RAILBOUNCE_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::RAILBOUNCE_Change()
{
	BasicAirAction();
}

void Actor::RAILBOUNCE_Update()
{
}

void Actor::RAILBOUNCE_UpdateSprite()
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

void Actor::RAILBOUNCE_TransitionToAction(int a)
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

void Actor::RAILBOUNCE_TimeIndFrameInc()
{

}

void Actor::RAILBOUNCE_TimeDepFrameInc()
{

}

int Actor::RAILBOUNCE_GetActionLength()
{
	return 8;
}

const char * Actor::RAILBOUNCE_GetTilesetName()
{
	return "hurt_64x64.png";
}