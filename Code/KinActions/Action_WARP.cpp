#include "Actor.h"
#include "Session.h"
#include "Enemy_Goal.h"
#include "ShipPickup.h"
//#include "VisualEffects.h"

using namespace sf;
using namespace std;

void Actor::WARP_Start()
{
	V2d goalPos = sess->GetLevelFinisherPos();

	V2d dir = normalize(goalPos - position);
	double speed = 80;
	double dist = length(goalPos - position);
	int numFrames = 120;
	if (dist > speed * numFrames)
	{
		speed = dist / numFrames;
	}

	velocity = dir * speed;

	if (sess->zoneTreeEnd != NULL && sess->currentZone != sess->zoneTreeEnd)
	{
		sess->WarpToZone(sess->zoneTreeEnd);
	}
}

void Actor::WARP_End()
{
	frame = 0;
}

void Actor::WARP_Change()
{
	double speed = length(velocity);

	V2d goalPos = sess->GetLevelFinisherPos();

	if (length(goalPos - position) < speed)
	{
		position = goalPos;
		velocity = V2d(0, 0);
		
		SetAction(GOALKILL);

		if (sess->goal != NULL)
		{
			sess->goal->HandleNoHealth();
		}
		//sess->ActivateZone(sess->zoneTreeEnd, true);
	}
}

void Actor::WARP_Update()
{
	
}

void Actor::WARP_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);

	if (facingRight)
	{
		double a = GetVectorAngleCW(normalize(velocity)) * 180 / PI;
		sprite->setRotation(a);
	}
	else
	{
		double a = GetVectorAngleCCW(normalize(velocity)) * 180 / PI;
		sprite->setRotation(-a + 180);
	}

	if (scorpOn)
		SetAerialScorpSprite();
}

void Actor::WARP_TransitionToAction(int a)
{

}

void Actor::WARP_TimeIndFrameInc()
{

}

void Actor::WARP_TimeDepFrameInc()
{

}

int Actor::WARP_GetActionLength()
{
	return -1; //infinite frames so I can count how long I'm in it
}

const char * Actor::WARP_GetTilesetName()
{
	return SPRINGSTUN_GetTilesetName();
}