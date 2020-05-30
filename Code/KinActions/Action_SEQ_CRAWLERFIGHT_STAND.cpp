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