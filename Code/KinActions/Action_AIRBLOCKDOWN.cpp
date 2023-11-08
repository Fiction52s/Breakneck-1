#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRBLOCKDOWN_Start()
{
	TryResetBlockCounter();
}

void Actor::AIRBLOCKDOWN_End()
{

}

void Actor::AIRBLOCKDOWN_Change()
{
	AirBlockChange();
}

void Actor::AIRBLOCKDOWN_Update()
{
	CheckHoldJump();

	++framesBlocking;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
	//hitstunFrames--;
}

void Actor::AIRBLOCKDOWN_UpdateSprite()
{
	UpdateAerialShieldSprite(9);
}

void Actor::AIRBLOCKDOWN_TransitionToAction(int a)
{
}

void Actor::AIRBLOCKDOWN_TimeIndFrameInc()
{

}

void Actor::AIRBLOCKDOWN_TimeDepFrameInc()
{

}

int Actor::AIRBLOCKDOWN_GetActionLength()
{
	return 1;
}

const char *Actor::AIRBLOCKDOWN_GetTilesetName()
{
	return "block_64x64.png";
}