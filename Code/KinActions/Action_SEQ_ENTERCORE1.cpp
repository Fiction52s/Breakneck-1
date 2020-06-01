#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_ENTERCORE1_Start()
{
}

void Actor::SEQ_ENTERCORE1_End()
{
	frame = actionLength[SEQ_ENTERCORE1] - 1;
}

void Actor::SEQ_ENTERCORE1_Change()
{
}

void Actor::SEQ_ENTERCORE1_Update()
{
}

void Actor::SEQ_ENTERCORE1_UpdateSprite()
{
	STAND_UpdateSprite();
}

void Actor::SEQ_ENTERCORE1_TransitionToAction(int a)
{

}

void Actor::SEQ_ENTERCORE1_TimeIndFrameInc()
{

}

void Actor::SEQ_ENTERCORE1_TimeDepFrameInc()
{

}

int Actor::SEQ_ENTERCORE1_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_ENTERCORE1_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}