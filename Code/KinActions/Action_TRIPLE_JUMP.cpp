#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::TRIPLE_JUMP_Start()
{
	//action = GetDoubleJump(); //a bit messy to use this
	//frame = 0;
	holdDouble = true;
}

void Actor::TRIPLE_JUMP_End()
{
	SetAction(JUMP);
	frame = 1;
}

void Actor::TRIPLE_JUMP_Change()
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

void Actor::TRIPLE_JUMP_Update()
{
	SetCurrHitboxes(homingHitboxes, 0);

	if (action == DOUBLE)
	{
		b.rh = doubleJumpHeight;
	}

	//	b.offset.y = -5;
	if (frame == 0)
	{
		ExecuteDoubleJump();
		ActivateEffect(PLAYERFX_BOUNCE_BOOST, Vector2f(position + V2d( 0, -80 )), 0, 30, 1, facingRight);
		ActivateSound(PlayerSounds::S_BOUNCEJUMP);
	}
	else
	{

		CheckHoldJump();


		AirMovement();
		//cout << PhantomResolve( owner->edges, owner->numPoints, V2d( 10, 0 ) ) << endl;

	}
}

void Actor::TRIPLE_JUMP_UpdateSprite()
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

void Actor::TRIPLE_JUMP_TransitionToAction(int a)
{

}

void Actor::TRIPLE_JUMP_TimeIndFrameInc()
{

}

void Actor::TRIPLE_JUMP_TimeDepFrameInc()
{

}

int Actor::TRIPLE_JUMP_GetActionLength()
{
	return 28 + 10;
}

const char * Actor::TRIPLE_JUMP_GetTilesetName()
{
	return "double_64x64.png";
}