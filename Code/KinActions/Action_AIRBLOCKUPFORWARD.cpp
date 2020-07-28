#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRBLOCKUPFORWARD_Start()
{
	TryResetBlockCounter();
}

void Actor::AIRBLOCKUPFORWARD_End()
{

}

void Actor::AIRBLOCKUPFORWARD_Change()
{
	AirBlockChange();
}

void Actor::AIRBLOCKUPFORWARD_Update()
{
	CheckHoldJump();

	++framesBlocking;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
}

void Actor::AIRBLOCKUPFORWARD_UpdateSprite()
{
	UpdateAerialShieldSprite(6);
}

void Actor::AIRBLOCKUPFORWARD_TransitionToAction(int a)
{
}

void Actor::AIRBLOCKUPFORWARD_TimeIndFrameInc()
{

}

void Actor::AIRBLOCKUPFORWARD_TimeDepFrameInc()
{

}

int Actor::AIRBLOCKUPFORWARD_GetActionLength()
{
	return 1;
}

Tileset * Actor::AIRBLOCKUPFORWARD_GetTileset()
{
	return GetActionTileset("block_64x64.png");
}