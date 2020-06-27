#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::DOUBLE_Start()
{
	//action = GetDoubleJump(); //a bit messy to use this
	//frame = 0;
	holdDouble = true;
}

void Actor::DOUBLE_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::DOUBLE_Change()
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

void Actor::DOUBLE_Update()
{
	if (action == DOUBLE)
	{
		//b.rh = doubleJumpHeight;
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

void Actor::DOUBLE_UpdateSprite()
{
	int fr = frame;
	if (frame > 27)
	{
		fr = 27;
	}

	SetSpriteTexture(action);

	SetSpriteTile(fr / 1, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::DOUBLE_TransitionToAction(int a)
{

}

void Actor::DOUBLE_TimeIndFrameInc()
{

}

void Actor::DOUBLE_TimeDepFrameInc()
{

}

int Actor::DOUBLE_GetActionLength()
{
	return 28 + 10;
}

Tileset * Actor::DOUBLE_GetTileset()
{
	return GetActionTileset("double_64x64.png");
}