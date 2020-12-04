#include "Actor.h"
#include "GameSession.h"
#include "EditorRail.h"

using namespace sf;
using namespace std;

void Actor::RAILSLIDE_Start()
{
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
		if (GameSession::IsWall(grindEdge->Normal()) < 0) //not wall
		{
			SetAction(JUMPSQUAT);
			//frame = 0;
		}
		else
		{
			hasDoubleJump = true;
			TryDoubleJump();
			grindEdge = NULL;
		}




		//if( abs( grindEdge->Normal().y ) )

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

Tileset * Actor::RAILSLIDE_GetTileset()
{
	return NULL;
}