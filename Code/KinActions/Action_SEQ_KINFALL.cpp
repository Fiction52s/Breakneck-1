#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_KINFALL_Start()
{
}

void Actor::SEQ_KINFALL_End()
{
	frame = 1;
}

void Actor::SEQ_KINFALL_Change()
{
}

void Actor::SEQ_KINFALL_Update()
{
}

void Actor::SEQ_KINFALL_UpdateSprite()
{
	JUMP_UpdateSprite();
}