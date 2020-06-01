#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::EXITWAIT_Start()
{
}

void Actor::EXITWAIT_End()
{
}

void Actor::EXITWAIT_Change()
{
}

void Actor::EXITWAIT_Update()
{
}

void Actor::EXITWAIT_UpdateSprite()
{
}

void Actor::EXITWAIT_TransitionToAction(int a)
{

}

void Actor::EXITWAIT_TimeIndFrameInc()
{

}

void Actor::EXITWAIT_TimeDepFrameInc()
{

}

int Actor::EXITWAIT_GetActionLength()
{
	return 1;
}

Tileset * Actor::EXITWAIT_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}