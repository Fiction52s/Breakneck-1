#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_KINSTAND_Start()
{
}

void Actor::SEQ_KINSTAND_End()
{
	frame = 0;
}

void Actor::SEQ_KINSTAND_Change()
{
}

void Actor::SEQ_KINSTAND_Update()
{
}

void Actor::SEQ_KINSTAND_UpdateSprite()
{
	STAND_UpdateSprite();
}