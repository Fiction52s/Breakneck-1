#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKDOWN_Start()
{
	TryResetBlockCounter();
}

void Actor::GROUNDBLOCKDOWN_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKDOWN_Change()
{
	GroundBlockChange();
}

void Actor::GROUNDBLOCKDOWN_Update()
{
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}

	ApplyBlockFriction();
}

void Actor::GROUNDBLOCKDOWN_UpdateSprite()
{
	SetSpriteTexture(action);

	UpdateGroundedShieldSprite(4);
}

void Actor::GROUNDBLOCKDOWN_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCKDOWN_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCKDOWN_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCKDOWN_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDBLOCKDOWN_GetTileset()
{
	return GetActionTileset("block_64x64.png");
}