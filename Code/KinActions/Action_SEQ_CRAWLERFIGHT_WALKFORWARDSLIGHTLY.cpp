#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Start()
{
}

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_End()
{
	frame = 0;
}

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Change()
{
}

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_Update()
{
	RunMovement();
}

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_UpdateSprite()
{
	RUN_UpdateSprite();
}

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TransitionToAction(int a)
{

}

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeIndFrameInc()
{

}

void Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_TimeDepFrameInc()
{

}

int Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetActionLength()
{
	return 10 * 4;
}

Tileset * Actor::SEQ_CRAWLERFIGHT_WALKFORWARDSLIGHTLY_GetTileset()
{
	return GetActionTileset("run_64x64.png");
}