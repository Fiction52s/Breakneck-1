#include "Actor.h"

using namespace sf;
using namespace std;

void Actor::GETSHARD_Start()
{
	//switch( )
	//SHARD_W1_0_GET_DASH_BOOST;
	//shardTypeMap["GET_AIRDASH_BOOST"] = SHARD_W1_1_GET_AIRDASH_BOOST;

}

void Actor::GETSHARD_End()
{
	frame = 0;
}

void Actor::GETSHARD_Change()
{
}

void Actor::GETSHARD_Update()
{
	if (frame == 0)
	{
		if (ground != NULL)
		{
			ground = NULL;
			framesInAir = 0;
			RestoreAirOptions();
		}
	}

	velocity = V2d(0, 0);
}

void Actor::GETSHARD_UpdateSprite()
{
	SetSpriteTexture(action);
	sprite->setPosition(position.x, position.y);
	SetSpriteTile(0, facingRight);
	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
}

void Actor::GETSHARD_TransitionToAction(int a)
{

}

void Actor::GETSHARD_TimeIndFrameInc()
{

}

void Actor::GETSHARD_TimeDepFrameInc()
{

}

int Actor::GETSHARD_GetActionLength()
{
	return 2;
}

Tileset * Actor::GETSHARD_GetTileset()
{
	return DEATH_GetTileset();
}