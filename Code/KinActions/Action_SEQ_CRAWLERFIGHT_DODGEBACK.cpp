#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Start()
{
}

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_End()
{
	frame = 1;
}

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Change()
{
}

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_Update()
{
	if (frame == 0)
	{
		//cout << "leaving ground" << endl;
		ground = NULL;
		velocity = V2d(-10, -20);

	}
}

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_UpdateSprite()
{
	JUMPSQUAT_UpdateSprite();
}

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TransitionToAction(int a)
{

}

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TimeIndFrameInc()
{

}

void Actor::SEQ_CRAWLERFIGHT_DODGEBACK_TimeDepFrameInc()
{

}

int Actor::SEQ_CRAWLERFIGHT_DODGEBACK_GetActionLength()
{
	return 2;
}

const char * Actor::SEQ_CRAWLERFIGHT_DODGEBACK_GetTilesetName()
{
	return "jump_64x64.png";
}