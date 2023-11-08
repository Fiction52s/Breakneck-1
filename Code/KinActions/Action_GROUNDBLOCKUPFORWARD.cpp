#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKUPFORWARD_Start()
{
	TryResetBlockCounter();
}

void Actor::GROUNDBLOCKUPFORWARD_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKUPFORWARD_Change()
{
	GroundBlockChange();
}

void Actor::GROUNDBLOCKUPFORWARD_Update()
{
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}

	ApplyBlockFriction();
}

void Actor::GROUNDBLOCKUPFORWARD_UpdateSprite()
{
	SetSpriteTexture(action);

	UpdateGroundedShieldSprite(1);
}

void Actor::GROUNDBLOCKUPFORWARD_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCKUPFORWARD_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCKUPFORWARD_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCKUPFORWARD_GetActionLength()
{
	return 1;
}

const char * Actor::GROUNDBLOCKUPFORWARD_GetTilesetName()
{
	return "block_64x64.png";
}