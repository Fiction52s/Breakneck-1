#include "Actor.h"
#include "GameSession.h"
#include "EditorRail.h"
#include "Wire.h"

using namespace sf;
using namespace std;

void Actor::RAILSLIDE_Start()
{
	RailPtr rail = grindEdge->rail;

	if (rail->GetRailType() == TerrainRail::LOCKED)
	{
		SetAction(LOCKEDRAILSLIDE);
		////should not be able to use wires at all while locked
		//if (rightWire != NULL)
		//	rightWire->Retract();

		//if (leftWire != NULL)
		//	leftWire->Retract();
	}
}

void Actor::RAILSLIDE_End()
{
	frame = 0;
}

void Actor::RAILSLIDE_Change()
{
	

	V2d along = grindEdge->Along();

	RailPtr rail = grindEdge->rail;

	if (JumpButtonPressed())
	{

		facingRight = IsRailSlideFacingRight();
		
		velocity = grindSpeed * along;

		SetAction(JUMP);
		frame = 0;

		regrindOffCount = 0;
		return;
	}

	if (TryAirDash())
	{
		facingRight = IsRailSlideFacingRight();
		velocity = grindSpeed * along;
		grindEdge = NULL;
		hasDoubleJump = true;
		return;
	}

	if (AirAttack())
	{
		facingRight = IsRailSlideFacingRight();
		
		velocity = grindSpeed * along;
		grindEdge = NULL;
		hasDoubleJump = true;
		hasAirDash = true;

		return;
	}
}

void Actor::RAILSLIDE_Update()
{
	RailGrindMovement();
}

void Actor::RAILSLIDE_UpdateSprite()
{
	GRINDBALL_UpdateSprite();

	bool r = grindSpeed > 0;

	if (scorpOn)
	{
		scorpSprite.setTexture(*ts_scorpSlide->texture);

		SetSpriteTile(&scorpSprite, ts_scorpSlide, 0, r);

		scorpSprite.setOrigin(scorpSprite.getLocalBounds().width / 2,
			scorpSprite.getLocalBounds().height / 2 + 10);
		scorpSprite.setPosition(position.x, position.y);
		scorpSprite.setRotation(sprite->getRotation());
		scorpSet = true;
	}
}

void Actor::RAILSLIDE_TransitionToAction(int a)
{

}

void Actor::RAILSLIDE_TimeIndFrameInc()
{

}

void Actor::RAILSLIDE_TimeDepFrameInc()
{

}

int Actor::RAILSLIDE_GetActionLength()
{
	return 1;
}

const char * Actor::RAILSLIDE_GetTilesetName()
{
	return NULL;
}