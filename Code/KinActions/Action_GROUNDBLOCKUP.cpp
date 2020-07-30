#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKUP_Start()
{
	TryResetBlockCounter();
}

void Actor::GROUNDBLOCKUP_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKUP_Change()
{
	GroundBlockChange();
}

void Actor::GROUNDBLOCKUP_Update()
{
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}

	ApplyBlockFriction();
}

void Actor::GROUNDBLOCKUP_UpdateSprite()
{
	SetSpriteTexture(action);

	UpdateGroundedShieldSprite(0);
}

void Actor::GROUNDBLOCKUP_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCKUP_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCKUP_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCKUP_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDBLOCKUP_GetTileset()
{
	return GetActionTileset("block_64x64.png");
}