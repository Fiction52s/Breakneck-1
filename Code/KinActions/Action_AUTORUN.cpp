#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::AUTORUN_Start()
{

}

void Actor::AUTORUN_End()
{
	frame = 0;
}

void Actor::AUTORUN_Change()
{

}

void Actor::AUTORUN_Update()
{
	RunMovement();
}

void Actor::AUTORUN_UpdateSprite()
{
	RUN_UpdateSprite();
}

void Actor::AUTORUN_TransitionToAction(int a)
{

}

void Actor::AUTORUN_TimeIndFrameInc()
{

}

void Actor::AUTORUN_TimeDepFrameInc()
{

}

int Actor::AUTORUN_GetActionLength()
{
	return RUN_GetActionLength();
}

const char * Actor::AUTORUN_GetTilesetName()
{
	return NULL;
}