#include "Actor.h"
#include "GameSession.h"

using namespace sf;
using namespace std;

void Actor::DEATH_Start()
{

}

void Actor::DEATH_End()
{
	
	frame = 0;
	dead = true;
}

void Actor::DEATH_Change()
{

}

void Actor::DEATH_Update()
{
	velocity.x = 0;
	velocity.y = 0;
	groundSpeed = 0;

	if (frame == GetActionLength(DEATH) - 1)
	{
		owner->NextFrameRestartLevel();
	}
}

void Actor::DEATH_UpdateSprite()
{
}

void Actor::DEATH_TransitionToAction(int a)
{

}

void Actor::DEATH_TimeIndFrameInc()
{

}

void Actor::DEATH_TimeDepFrameInc()
{

}

int Actor::DEATH_GetActionLength()
{
	return 44 * 2;
}

Tileset * Actor::DEATH_GetTileset()
{
	return GetActionTileset("death_128x96.png");
}