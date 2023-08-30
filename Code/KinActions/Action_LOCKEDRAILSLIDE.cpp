#include "Actor.h"
#include "GameSession.h"
#include "EditorRail.h"
#include "Wire.h"

using namespace sf;
using namespace std;

void Actor::LOCKEDRAILSLIDE_Start()
{
	RailPtr rail = grindEdge->rail;
	//if (rail->GetRailType() == TerrainRail::LOCKED)
	//{
	//	//should not be able to use wires at all while locked
	//	if (rightWire != NULL)
	//		rightWire->Retract();

	//	if (leftWire != NULL)
	//		leftWire->Retract();
	//}

	double minLockedRailSlideVel = 30;

	grindSpeed = minLockedRailSlideVel;
	return;

	
	if (abs(grindSpeed < minLockedRailSlideVel))
	{
		if (grindSpeed > 0)
		{
			grindSpeed = minLockedRailSlideVel;
		}
		else if (grindSpeed < 0)
		{
			grindSpeed = -minLockedRailSlideVel;
		}
	}
}

void Actor::LOCKEDRAILSLIDE_End()
{
	frame = 0;
}

void Actor::LOCKEDRAILSLIDE_Change()
{
	//V2d along = grindEdge->Along();

	//RailPtr rail = grindEdge->rail;

	//if (rail->GetRailType() == TerrainRail::LOCKED)
	//{
	//	return;
	//}

	//if (JumpButtonPressed())
	//{

	//	facingRight = IsRailSlideFacingRight();
	//	velocity = grindSpeed * along;
	//	if (GameSession::IsWall(grindEdge->Normal()) < 0) //not wall
	//	{
	//		SetAction(JUMPSQUAT);
	//		//frame = 0;
	//	}
	//	else
	//	{
	//		hasDoubleJump = true;
	//		TryDoubleJump();
	//		grindEdge = NULL;
	//	}




	//	//if( abs( grindEdge->Normal().y ) )

	//	frame = 0;

	//	regrindOffCount = 0;
	//	return;
	//}

	//if (TryAirDash())
	//{
	//	facingRight = IsRailSlideFacingRight();
	//	velocity = grindSpeed * along;
	//	grindEdge = NULL;
	//	hasDoubleJump = true;
	//	return;
	//}

	//if (AirAttack())
	//{
	//	facingRight = IsRailSlideFacingRight();

	//	velocity = grindSpeed * along;
	//	grindEdge = NULL;
	//	hasDoubleJump = true;
	//	hasAirDash = true;

	//	return;
	//}
}

void Actor::LOCKEDRAILSLIDE_Update()
{
	RailGrindMovement();
}

void Actor::LOCKEDRAILSLIDE_UpdateSprite()
{
	GRINDBALL_UpdateSprite();
}

void Actor::LOCKEDRAILSLIDE_TransitionToAction(int a)
{

}

void Actor::LOCKEDRAILSLIDE_TimeIndFrameInc()
{

}

void Actor::LOCKEDRAILSLIDE_TimeDepFrameInc()
{

}

int Actor::LOCKEDRAILSLIDE_GetActionLength()
{
	return 1;
}

Tileset * Actor::LOCKEDRAILSLIDE_GetTileset()
{
	return NULL;
}