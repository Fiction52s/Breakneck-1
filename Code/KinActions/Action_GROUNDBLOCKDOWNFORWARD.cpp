#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GROUNDBLOCKDOWNFORWARD_Start()
{
	if (!IsBlockAction(oldAction))
	{
		framesBlocking = 0;
	}

	
}

void Actor::GROUNDBLOCKDOWNFORWARD_End()
{
	frame = 0;
}

void Actor::GROUNDBLOCKDOWNFORWARD_Change()
{
	GroundBlockChange();
}

void Actor::GROUNDBLOCKDOWNFORWARD_Update()
{
	framesBlocking++;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
}

void Actor::GROUNDBLOCKDOWNFORWARD_UpdateSprite()
{
	SetSpriteTexture(action);

	UpdateGroundedShieldSprite(3);
}

void Actor::GROUNDBLOCKDOWNFORWARD_TransitionToAction(int a)
{
}

void Actor::GROUNDBLOCKDOWNFORWARD_TimeIndFrameInc()
{

}

void Actor::GROUNDBLOCKDOWNFORWARD_TimeDepFrameInc()
{

}

int Actor::GROUNDBLOCKDOWNFORWARD_GetActionLength()
{
	return 1;
}

Tileset * Actor::GROUNDBLOCKDOWNFORWARD_GetTileset()
{
	return GetActionTileset("block_64x64.png");
}