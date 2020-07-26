#include "Actor.h"
#include "Session.h"
#include "MapHeader.h"
#include "GameMode.h"
#include "Enemy_Bird.h"
#include <iostream>
#include "Sequence.h"

using namespace sf;
using namespace std;

void Actor::SUPERBIRD_Start()
{
	if (sess->superSequence != NULL)
	{
		sess->superSequence->Reset();
		sess->SetActiveSequence(sess->superSequence);

		sess->Pause(3 * 16);
		if (sess->GetGameMode() == MapHeader::T_FIGHT)
		{
			FightMode *fm = (FightMode*)sess->gameMode;

			if (fm->testBird->spawned)
			{
				sess->RemoveEnemy(fm->testBird);
			}

			
			int pIndex = 0;
			fm->testBird->playerIndex = pIndex;

			fm->testBird->targetPlayerIndex = pIndex;
			fm->testBird->startPosInfo.position = sess->GetPlayerPos(pIndex);
			fm->testBird->Reset();

			sess->AddEnemy(fm->testBird);
		}
	}
}

void Actor::SUPERBIRD_End()
{
	//this is an aerial super
	SetAction(JUMP);
	frame = 1;
}

void Actor::SUPERBIRD_Change()
{
}

void Actor::SUPERBIRD_Update()
{
	if (frame == 0 && slowCounter == 1)
	{
		//cout << "pausing for 30" << endl;

	}
}

void Actor::SUPERBIRD_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
}

void Actor::SUPERBIRD_TransitionToAction(int a)
{

}

void Actor::SUPERBIRD_TimeIndFrameInc()
{

}

void Actor::SUPERBIRD_TimeDepFrameInc()
{

}

int Actor::SUPERBIRD_GetActionLength()
{
	return 1;
}

Tileset * Actor::SUPERBIRD_GetTileset()
{
	return SLIDE_GetTileset();
}