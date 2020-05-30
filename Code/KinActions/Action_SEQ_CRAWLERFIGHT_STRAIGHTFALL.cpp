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