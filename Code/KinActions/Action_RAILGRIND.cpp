#include "Actor.h"
#include "EditorRail.h"
#include "Gamesession.h"

using namespace sf;
using namespace std;

void Actor::RAILGRIND_Start()
{
}

void Actor::RAILGRIND_End()
{
	frame = 0;
}

void Actor::RAILGRIND_Change()
{
	RailPtr rail = grindEdge->rail;
	if (currInput.A && !prevInput.A)
	{
		if (GameSession::IsWall(grindEdge->Normal()) < 0) //not wall
		{
			SetActionExpr(JUMPSQUAT);
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
	else if (currInput.B && !prevInput.B)
	{
		SetAction(RAILDASH);
		frame = 0;
		grindEdge = NULL;
	}
	if (currInput.Y && !prevInput.Y && framesGrinding > 1)
	{
		SetAction(JUMP);
		grindEdge = NULL;
		frame = 1;
		regrindOffCount = 0;
		return;
	}
}

void Actor::RAILGRIND_Update()
{
	RailGrindMovement();
}

void Actor::RAILGRIND_UpdateSprite()
{
	GRINDBALL_UpdateSprite();
}

void Actor::RAILGRIND_TransitionToAction(int a)
{

}

void Actor::RAILGRIND_TimeIndFrameInc()
{

}

void Actor::RAILGRIND_TimeDepFrameInc()
{

}

int Actor::RAILGRIND_GetActionLength()
{
	return 1;
}

Tileset * Actor::RAILGRIND_GetTileset()
{
	return NULL;
}