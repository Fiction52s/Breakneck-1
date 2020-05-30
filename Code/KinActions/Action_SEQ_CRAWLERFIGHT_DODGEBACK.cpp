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