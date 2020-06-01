#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_KINSTAND_Start()
{
}

void Actor::SEQ_KINSTAND_End()
{
	frame = 0;
}

void Actor::SEQ_KINSTAND_Change()
{
}

void Actor::SEQ_KINSTAND_Update()
{
}

void Actor::SEQ_KINSTAND_UpdateSprite()
{
	STAND_UpdateSprite();
}

void Actor::SEQ_KINSTAND_TransitionToAction(int a)
{

}

void Actor::SEQ_KINSTAND_TimeIndFrameInc()
{

}

void Actor::SEQ_KINSTAND_TimeDepFrameInc()
{

}

int Actor::SEQ_KINSTAND_GetActionLength()
{
	return STAND_GetActionLength();
}

Tileset * Actor::SEQ_KINSTAND_GetTileset()
{
	return NULL;
}