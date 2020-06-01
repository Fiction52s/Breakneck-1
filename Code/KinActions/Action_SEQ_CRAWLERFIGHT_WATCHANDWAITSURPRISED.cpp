#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Start()
{
}

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_End()
{
	frame = 0;
}

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Change()
{
}

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_Update()
{
}

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_UpdateSprite()
{
	SLIDE_UpdateSprite();
}

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TransitionToAction(int a)
{

}

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeIndFrameInc()
{

}

void Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_TimeDepFrameInc()
{

}

int Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetActionLength()
{
	return 1;
}

Tileset * Actor::SEQ_CRAWLERFIGHT_WATCHANDWAITSURPRISED_GetTileset()
{
	return GetActionTileset("slide_64x64.png");
}