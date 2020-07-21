#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRBLOCK_Start()
{
	if (!IsBlockAction(oldAction))
	{
		framesBlocking = 0;
	}
}

void Actor::AIRBLOCK_End()
{

}

void Actor::AIRBLOCK_Change()
{
	if (currInput.LLeft())
	{
		facingRight = false;
	}
	else if (currInput.LRight())
	{
		facingRight = true;
	}

	if (!currInput.Y && blockstunFrames == 0)
	{
		SetAction(JUMP);
		frame = 1;
	}
	else
	{
		if (TryDoubleJump()) return;

		if (AirAttack()) return;
	}
}

void Actor::AIRBLOCK_Update()
{
	++framesBlocking;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
	//hitstunFrames--;
}

void Actor::AIRBLOCK_UpdateSprite()
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

void Actor::AIRBLOCK_TransitionToAction(int a)
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

void Actor::AIRBLOCK_TimeIndFrameInc()
{

}

void Actor::AIRBLOCK_TimeDepFrameInc()
{

}

int Actor::AIRBLOCK_GetActionLength()
{
	return 1;
}

Tileset * Actor::AIRBLOCK_GetTileset()
{
	return GetActionTileset("block_air_f_64x64.png");
}