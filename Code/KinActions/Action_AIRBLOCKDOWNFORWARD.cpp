#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRBLOCKDOWNFORWARD_Start()
{
	TryResetBlockCounter();
}

void Actor::AIRBLOCKDOWNFORWARD_End()
{

}

void Actor::AIRBLOCKDOWNFORWARD_Change()
{
	AirBlockChange();
}

void Actor::AIRBLOCKDOWNFORWARD_Update()
{
	CheckHoldJump();

	++framesBlocking;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
	//hitstunFrames--;
}

void Actor::AIRBLOCKDOWNFORWARD_UpdateSprite()
{
	UpdateAerialShieldSprite(8);
}

void Actor::AIRBLOCKDOWNFORWARD_TransitionToAction(int a)
{
}

void Actor::AIRBLOCKDOWNFORWARD_TimeIndFrameInc()
{

}

void Actor::AIRBLOCKDOWNFORWARD_TimeDepFrameInc()
{

}

int Actor::AIRBLOCKDOWNFORWARD_GetActionLength()
{
	return 1;
}

Tileset * Actor::AIRBLOCKDOWNFORWARD_GetTileset()
{
	return GetActionTileset("block_64x64.png");
}