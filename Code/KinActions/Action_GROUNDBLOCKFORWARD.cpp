#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKFORWARD_Start()
{
	TryResetBlockCounter();
}

void Actor::GROUNDBLOCKFORWARD_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKFORWARD_Change()
{
	GroundBlockChange();
}

void Actor::GROUNDBLOCKFORWARD_Update()
{
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}

	ApplyBlockFriction();
}

void Actor::GROUNDBLOCKFORWARD_UpdateSprite()
{
	SetSpriteTexture(action);

	UpdateGroundedShieldSprite(2);
}

void Actor::GROUNDBLOCKFORWARD_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCKFORWARD_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCKFORWARD_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCKFORWARD_GetActionLength()
{
	return 1;
}

const char * Actor::GROUNDBLOCKFORWARD_GetTilesetName()
{
	return "block_64x64.png";
}