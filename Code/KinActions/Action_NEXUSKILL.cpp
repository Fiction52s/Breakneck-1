#include "Actor.h"
#include "HUD.h"
#include "Wire.h"
#include "MainMenu.h"
#include "AdventureManager.h"

using namespace sf;
using namespace std;

void Actor::NEXUSKILL_Start()
{
	sess->totalFramesBeforeGoal = sess->totalGameFrames;

	SetExpr(KinMask::Expr_NEUTRAL);
	velocity = V2d(0, 0);
	groundSpeed = 0;
	grindSpeed = 0;

	position = sess->goalNodePos;
	rightWire->Reset();
	leftWire->Reset();

	SetKinMode(K_NORMAL);

	bool setRecord = false;
	if (owner != NULL)
	{
		if (owner->mainMenu->gameRunType == MainMenu::GRT_ADVENTURE)
		{
			if (!owner->IsReplayOn())
			{
				setRecord = adventureManager->CompleteCurrentMap(owner);
			}
		}
	}

	if (setRecord)
	{
		owner->scoreDisplay->madeRecord = true;
	}
}

void Actor::NEXUSKILL_End()
{
	SetAction(SEQ_FLOAT_TO_NEXUS_OPENING);
	frame = 0;
}

void Actor::NEXUSKILL_Change()
{
}

void Actor::NEXUSKILL_Update()
{
}

void Actor::NEXUSKILL_UpdateSprite()
{
	GOALKILL_UpdateSprite();
}

void Actor::NEXUSKILL_TransitionToAction(int a)
{

}

void Actor::NEXUSKILL_TimeIndFrameInc()
{

}

void Actor::NEXUSKILL_TimeDepFrameInc()
{

}

int Actor::NEXUSKILL_GetActionLength()
{
	return 63 * 2;
}

Tileset * Actor::NEXUSKILL_GetTileset()
{
	return NULL;
}