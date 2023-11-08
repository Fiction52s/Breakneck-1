#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::WIREHOLD_Start()
{
}

void Actor::WIREHOLD_End()
{
	frame = 0;
}

void Actor::WIREHOLD_Change()
{
}

void Actor::WIREHOLD_Update()
{
}

void Actor::WIREHOLD_UpdateSprite()
{
}

void Actor::WIREHOLD_TransitionToAction(int a)
{

}

void Actor::WIREHOLD_TimeIndFrameInc()
{

}

void Actor::WIREHOLD_TimeDepFrameInc()
{

}

int Actor::WIREHOLD_GetActionLength()
{
	return 1;
}

const char * Actor::WIREHOLD_GetTilesetName()
{
	return STEEPSLIDE_GetTilesetName();
}