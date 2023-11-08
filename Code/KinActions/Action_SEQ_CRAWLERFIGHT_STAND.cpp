#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_CRAWLERFIGHT_STAND_Start()
{
}

void Actor::SEQ_CRAWLERFIGHT_STAND_End()
{
	frame = 0;
}

void Actor::SEQ_CRAWLERFIGHT_STAND_Change()
{
}

void Actor::SEQ_CRAWLERFIGHT_STAND_Update()
{
}

void Actor::SEQ_CRAWLERFIGHT_STAND_UpdateSprite()
{
	STAND_UpdateSprite();
}

void Actor::SEQ_CRAWLERFIGHT_STAND_TransitionToAction(int a)
{

}

void Actor::SEQ_CRAWLERFIGHT_STAND_TimeIndFrameInc()
{

}

void Actor::SEQ_CRAWLERFIGHT_STAND_TimeDepFrameInc()
{

}

int Actor::SEQ_CRAWLERFIGHT_STAND_GetActionLength()
{
	return 20 * 8;
}

const char * Actor::SEQ_CRAWLERFIGHT_STAND_GetTilesetName()
{
	return "stand_64x64.png";
}