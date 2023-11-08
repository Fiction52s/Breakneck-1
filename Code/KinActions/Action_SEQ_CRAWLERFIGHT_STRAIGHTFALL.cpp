#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Start()
{
}

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_End()
{
	frame = 1;
}

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Change()
{
}

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_Update()
{
}

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_UpdateSprite()
{
	JUMP_UpdateSprite();
}

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TransitionToAction(int a)
{

}

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeIndFrameInc()
{

}

void Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_TimeDepFrameInc()
{

}

int Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetActionLength()
{
	return 2;
}

const char * Actor::SEQ_CRAWLERFIGHT_STRAIGHTFALL_GetTilesetName()
{
	return "jump_64x64.png";
}