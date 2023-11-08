#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRBLOCKFORWARD_Start()
{
	TryResetBlockCounter();
}

void Actor::AIRBLOCKFORWARD_End()
{

}

void Actor::AIRBLOCKFORWARD_Change()
{
	AirBlockChange();
}

void Actor::AIRBLOCKFORWARD_Update()
{
	CheckHoldJump();

	++framesBlocking;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
	//hitstunFrames--;
}

void Actor::AIRBLOCKFORWARD_UpdateSprite()
{
	UpdateAerialShieldSprite(7);
}

void Actor::AIRBLOCKFORWARD_TransitionToAction(int a)
{
}

void Actor::AIRBLOCKFORWARD_TimeIndFrameInc()
{

}

void Actor::AIRBLOCKFORWARD_TimeDepFrameInc()
{

}

int Actor::AIRBLOCKFORWARD_GetActionLength()
{
	return 1;
}

const char * Actor::AIRBLOCKFORWARD_GetTilesetName()
{
	return "block_64x64.png";
}