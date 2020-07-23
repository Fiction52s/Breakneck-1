#include "Actor.h"
#include "HUD.h"

using namespace sf;
using namespace std;

void Actor::AIRBLOCKUP_Start()
{
	TryResetBlockCounter();
}

void Actor::AIRBLOCKUP_End()
{

}

void Actor::AIRBLOCKUP_Change()
{
	AirBlockChange();
}

void Actor::AIRBLOCKUP_Update()
{
	CheckHoldJump();

	++framesBlocking;

	if (blockstunFrames > 0)
	{
		blockstunFrames--;
	}
	//hitstunFrames--;
}

void Actor::AIRBLOCKUP_UpdateSprite()
{
	UpdateAerialShieldSprite(5);
}

void Actor::AIRBLOCKUP_TransitionToAction(int a)
{
	stunBufferedJump = false;
	stunBufferedDash = false;
	stunBufferedAttack = Action::Count;
}

void Actor::AIRBLOCKUP_TimeIndFrameInc()
{

}

void Actor::AIRBLOCKUP_TimeDepFrameInc()
{

}

int Actor::AIRBLOCKUP_GetActionLength()
{
	return 1;
}

Tileset * Actor::AIRBLOCKUP_GetTileset()
{
	return GetActionTileset("block_64x64.png");
}