#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_CRAWLERFIGHT_LAND_Start()
{
}

void Actor::SEQ_CRAWLERFIGHT_LAND_End()
{
	frame = 0;
	SetAction(SEQ_CRAWLERFIGHT_STAND);
}

void Actor::SEQ_CRAWLERFIGHT_LAND_Change()
{
}

void Actor::SEQ_CRAWLERFIGHT_LAND_Update()
{
}

void Actor::SEQ_CRAWLERFIGHT_LAND_UpdateSprite()
{
	LAND_UpdateSprite();
}

void Actor::SEQ_CRAWLERFIGHT_LAND_TransitionToAction(int a)
{

}

void Actor::SEQ_CRAWLERFIGHT_LAND_TimeIndFrameInc()
{

}

void Actor::SEQ_CRAWLERFIGHT_LAND_TimeDepFrameInc()
{

}

int Actor::SEQ_CRAWLERFIGHT_LAND_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_CRAWLERFIGHT_LAND_GetTileset()
{
	return GetActionTileset("land_64x64.png");
}