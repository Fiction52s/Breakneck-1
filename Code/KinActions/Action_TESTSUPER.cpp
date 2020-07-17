#include "Actor.h"
#include "Session.h"
#include "MapHeader.h"
#include "GameMode.h"
#include "Enemy_Gator.h"
#include <iostream>
#include "Sequence.h"

using namespace sf;
using namespace std;

void Actor::TESTSUPER_Start()
{
	//Tileset *ts = sess->GetSizedTileset("Bosses/Gator/super_lightning_960x540.png");
	if (sess->superSequence != NULL)
	{
		sess->superSequence->Reset();
		sess->SetActiveSequence(sess->superSequence);

		sess->Pause(30);
		if (sess->GetGameMode() == MapHeader::T_FIGHT)
		{
			FightMode *fm = (FightMode*)sess->gameMode;

			if (fm->testGator->spawned)
			{
				sess->RemoveEnemy(fm->testGator);
			}
			fm->testGator->playerIndex = 1;
			fm->testGator->startPosInfo.position = position;
			fm->testGator->Reset();

			sess->AddEnemy(fm->testGator);
		}
	}
}

void Actor::TESTSUPER_End()
{
	//this is an aerial super
	SetAction(JUMP);
	frame = 1;
}

void Actor::TESTSUPER_Change()
{
}

void Actor::TESTSUPER_Update()
{
	if (frame == 0 && slowCounter == 1)
	{
		//cout << "pausing for 30" << endl;
		
	}
}

void Actor::TESTSUPER_UpdateSprite()
{
	SetSpriteTexture(action);

	SetSpriteTile(0, facingRight);

	sprite->setOrigin(sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
	sprite->setPosition(position.x, position.y);
}

void Actor::TESTSUPER_TransitionToAction(int a)
{

}

void Actor::TESTSUPER_TimeIndFrameInc()
{

}

void Actor::TESTSUPER_TimeDepFrameInc()
{

}

int Actor::TESTSUPER_GetActionLength()
{
	return 1;
}

Tileset * Actor::TESTSUPER_GetTileset()
{
	return SLIDE_GetTileset();
}