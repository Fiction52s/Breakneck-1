#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::BOOSTERBOUNCE_Start()
{
	SetExpr(KinMask::Expr::Expr_HURT);
}

void Actor::BOOSTERBOUNCE_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::BOOSTERBOUNCE_Change()
{
	BasicAirAction();
}

void Actor::BOOSTERBOUNCE_Update()
{
}

void Actor::BOOSTERBOUNCE_UpdateSprite()
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

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::BOOSTERBOUNCE_TransitionToAction(int a)
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

void Actor::BOOSTERBOUNCE_TimeIndFrameInc()
{

}

void Actor::BOOSTERBOUNCE_TimeDepFrameInc()
{

}

int Actor::BOOSTERBOUNCE_GetActionLength()
{
	return 20;
}

const char * Actor::BOOSTERBOUNCE_GetTilesetName()
{
	return "hurt_64x64.png";
}