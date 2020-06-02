#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::BACKWARDSDOUBLE_Start()
{
	//action = GetDoubleJump();
	//frame = 0;
	holdDouble = true;
}

void Actor::BACKWARDSDOUBLE_End()
{
	SetActionExpr(JUMP);
	frame = 1;
}

void Actor::BACKWARDSDOUBLE_Change()
{
	if ((frame == 1 || (frame == 0 && slowCounter > 1)) && doubleJumpBufferedAttack != DOUBLE)
	{
		SetAction(doubleJumpBufferedAttack);
		doubleJumpBufferedAttack = DOUBLE;
		frame = 0;
		return;
	}
	BasicAirAction();
}

void Actor::BACKWARDSDOUBLE_Update()
{
	if (action == DOUBLE)
	{
		b.rh = doubleJumpHeight;
	}

	//	b.offset.y = -5;
	if (frame == 0)
	{
		ExecuteDoubleJump();
	}
	else
	{

		CheckHoldJump();


		AirMovement();
		//cout << PhantomResolve( owner->edges, owner->numPoints, V2d( 10, 0 ) ) << endl;

	}
}

void Actor::BACKWARDSDOUBLE_UpdateSprite()
{
	int fr = frame;

	SetSpriteTexture(action);
	SetSpriteTile(fr / 1, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::BACKWARDSDOUBLE_TransitionToAction(int a)
{

}

void Actor::BACKWARDSDOUBLE_TimeIndFrameInc()
{

}

void Actor::BACKWARDSDOUBLE_TimeDepFrameInc()
{

}

int Actor::BACKWARDSDOUBLE_GetActionLength()
{
	return 40;
}

Tileset * Actor::BACKWARDSDOUBLE_GetTileset()
{
	return GetActionTileset("double_back_96x96.png");
}