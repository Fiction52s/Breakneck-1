#include "Actor.h"
#include "Editsession.h" //testing
#include "GateMarker.h"

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
	BasicGroundAction(currNormal);
}

void Actor::STAND_Update()
{
	groundSpeed = 0;

	if (framesStanding == 120)
	{
		if (editOwner != NULL)
		{
			editOwner->testGateMarker->FadeIn();
		}
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

	double angle = GroundedAngle();

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);

	V2d pp = ground->GetPosition(edgeQuantity);

	if ((angle == 0 && !reversed) || (approxEquals(angle, PI) && reversed))
		sprite->setPosition(pp.x + offsetX, pp.y);
	else
		sprite->setPosition(pp.x, pp.y);
	sprite->setRotation(angle / PI * 180);

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

}

void Actor::STAND_TimeIndFrameInc()
{

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

void Actor::STAND_TransitionToAction(int a)
{

}

void Actor::STAND_TimeIndFrameInc()
{

}

void Actor::STAND_TimeDepFrameInc()
{

}

int Actor::STAND_GetActionLength()
{
	return 1;
}

Tileset * Actor::STAND_GetTileset()
{
	return GetActionTileset("hurt_64x64.png");
}