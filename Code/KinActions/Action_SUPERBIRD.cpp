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
		sess->superSequence->SetSuperType(SuperSequence::SuperType::BIRD);
		sess->SetActiveSequence(sess->superSequence);

		sess->Pause(3 * 16);
		if (sess->gameModeType == MatchParams::GAME_MODE_FIGHT)
		{
			FightMode *fm = (FightMode*)sess->gameMode;

			if (fm->testBird->spawned)
			{
				sess->RemoveEnemy(fm->testBird);
			}

			sess->currSuperPlayer = this;
			currBirdCommandIndex = 0;
			
			int pIndex = 1;
			fm->testBird->playerIndex = pIndex;

			birdCommands[0]->action = 0;
			birdCommands[0]->facingRight = true;

			birdCommands[1]->action = 0;
			birdCommands[1]->facingRight = false;

			birdCommands[2]->action = 0;
			birdCommands[2]->facingRight = true;

			for (int i = 0; i < 3; ++i)
			{
				//fm->testBird->SetCommand(i, *birdCommands[i]);
			}

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

const char * Actor::SUPERBIRD_GetTilesetName()
{
	return SLIDE_GetTilesetName();
}