#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SPAWNWAIT_Start()
{
}

void Actor::SPAWNWAIT_End()
{
	SetAction(INTRO);
	frame = 0;
}

void Actor::SPAWNWAIT_Change()
{
}

void Actor::SPAWNWAIT_Update()
{
}

void Actor::SPAWNWAIT_UpdateSprite()
{
}

void Actor::SPAWNWAIT_TransitionToAction(int a)
{

}

void Actor::SPAWNWAIT_TimeIndFrameInc()
{

}

void Actor::SPAWNWAIT_TimeDepFrameInc()
{

}

int Actor::SPAWNWAIT_GetActionLength()
{
	return 60;
}

const char * Actor::SPAWNWAIT_GetTilesetName()
{
	return NULL;
}