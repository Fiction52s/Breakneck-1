#include "Actor.h"
#include "GameSession.h"
#include "HUD.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

void Actor::DEATH_Start()
{
	SetExpr(KinMask::Expr_DEATHYELL);
}

void Actor::DEATH_End()
{
	frame = 0;
	dead = true;
}

void Actor::DEATH_Change()
{

}

void Actor::DEATH_Update()
{
	velocity.x = 0;
	velocity.y = 0;
	groundSpeed = 0;

	int myActionLength = DEATH_GetActionLength();

	if (frame == myActionLength - 56) //66 because 11 frames, mult of 6
	{
		SetExpr(KinMask::Expr_DEATH);
	}
	

	if (frame == myActionLength - 1 )
	{
		if (owner != NULL)
		{
			//owner->NextFrameRestartLevel();
			if (owner->gameModeType == MatchParams::GAME_MODE_FIGHT)
			{
				SetAction(HIDDEN);
				dead = true; //lets hope this doesnt mess with anything
			}
			else
			{
				owner->RestartGame();
			}
		}
		else if (editOwner != NULL)
		{
			editOwner->TestPlayerMode();
		}
	}
	
}

void Actor::DEATH_UpdateSprite()
{
}

void Actor::DEATH_TransitionToAction(int a)
{

}

void Actor::DEATH_TimeIndFrameInc()
{

}

void Actor::DEATH_TimeDepFrameInc()
{

}

int Actor::DEATH_GetActionLength()
{
	return 44 * 2;
}

const char * Actor::DEATH_GetTilesetName()
{
	return "death_64x64.png";
}