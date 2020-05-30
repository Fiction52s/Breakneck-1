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