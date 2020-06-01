#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_KINFALL_Start()
{
}

void Actor::SEQ_KINFALL_End()
{
	frame = 1;
}

void Actor::SEQ_KINFALL_Change()
{
}

void Actor::SEQ_KINFALL_Update()
{
}

void Actor::SEQ_KINFALL_UpdateSprite()
{
	JUMP_UpdateSprite();
}

void Actor::SEQ_KINFALL_TransitionToAction(int a)
{

}

void Actor::SEQ_KINFALL_TimeIndFrameInc()
{

}

void Actor::SEQ_KINFALL_TimeDepFrameInc()
{

}

int Actor::SEQ_KINFALL_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_KINFALL_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}