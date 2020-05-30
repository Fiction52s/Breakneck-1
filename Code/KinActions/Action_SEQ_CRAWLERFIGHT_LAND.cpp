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