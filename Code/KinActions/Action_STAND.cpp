#include "Actor.h"
#include "Editsession.h" //testing
#include "GateMarker.h"
#include "GameSession.h"

using namespace sf;
using namespace std;

void Actor::STAND_Start()
{
	framesStanding = 0;
}

void Actor::STAND_End()
{
	frame = 0;
}

void Actor::STAND_Change()
{
	BasicGroundAction();
}

void Actor::STAND_Update()
{
	groundSpeed = 0;

	if (framesStanding == 30)
	{
		if (owner != NULL)
		{
			//owner->gateMarkers->FadeIn();
		}
		/*if (editOwner != NULL)
		{
			editOwner->testGateMarker->FadeIn();
		}*/
	}
}

void Actor::STAND_UpdateSprite()
{
	SetSpriteTexture(STAND);

	//the %20 is for seq
	bool r = (facingRight && !reversed) || (!facingRight && reversed);
	int f = (frame / 8) % 20;
	SetSpriteTile(f, r);
	//assert(ground != NULL);

	SetGroundedSpriteTransform();

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpStand->texture);

		SetSpriteTile(&scorpSprite, ts_scorpStand, f, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 10);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::STAND_TransitionToAction(int a)
{
	if (framesStanding >= 30)
	{
		if (owner != NULL)
		{
			//owner->gateMarkers->FadeOut();
		}
		//editOwner->testGateMarker->FadeOut();
	}
}

void Actor::STAND_TimeIndFrameInc()
{
	
	++framesStanding;
}

void Actor::STAND_TimeDepFrameInc()
{

}

int Actor::STAND_GetActionLength()
{
	return 20 * 8;
}

Tileset * Actor::STAND_GetTileset()
{
	return GetActionTileset("stand_64x64.png");
}