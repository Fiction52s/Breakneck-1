#include "Actor.h"
#include "SoundTypes.h"

using namespace sf;
using namespace std;

void Actor::WALLCLING_Start()
{
	ActivateRepeatingSound(PlayerSounds::S_WALLSLIDE, true);
}

void Actor::WALLCLING_End()
{
	frame = 0;
}

void Actor::WALLCLING_Change()
{
	if (!currInput.LDown() && ((facingRight && currInput.LRight()) || (!facingRight && currInput.LLeft())))
	{
		SetAction(WALLJUMP);
		frame = 0;
	}
	else if (TryDoubleJump())
	{
		return;
	}
	else if (currInput.LDown())
	{
		SetAction(JUMP);
		frame = 1;
		holdJump = false;
	}
	else if (currInput.rightShoulder && !prevInput.rightShoulder)
	{
		SetAction(WALLATTACK);
		frame = 0;
	}
}

void Actor::WALLCLING_Update()
{
	directionalInputFreezeFrames = 0;
	if (velocity.y > clingSpeed)
	{
		//cout << "running wallcling" << endl;
		velocity.y = clingSpeed;
	}
	AirMovement();
}

void Actor::WALLCLING_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
	sprite->setRotation(0);
}

void Actor::WALLCLING_TransitionToAction(int a)
{

}

void Actor::WALLCLING_TimeIndFrameInc()
{

}

void Actor::WALLCLING_TimeDepFrameInc()
{

}

int Actor::WALLCLING_GetActionLength()
{
	return 1;
}

const char * Actor::WALLCLING_GetTilesetName()
{
	return "wall_cling_64x64.png";
}