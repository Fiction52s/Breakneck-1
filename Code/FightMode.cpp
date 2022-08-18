#include "Session.h"
#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"
#include "Enemy_Bird.h"
#include "FightEndSequence.h"
#include "NetplayManager.h"

using namespace std;
using namespace sf;

FightMode::FightMode()
{
	/*gatorParams = new BasicAirEnemyParams(sess->types["gator"], 1);
	gatorParams->CreateMyEnemy();
	testGator = (Gator*)gatorParams->myEnemy;

	birdParams = new BasicAirEnemyParams(sess->types["bird"], 1);
	birdParams->CreateMyEnemy();
	testBird = (Bird*)birdParams->myEnemy;*/

	endSeq = new FightEndSequence;
	endSeq->Init();
}

FightMode::~FightMode()
{
	//delete gatorParams;
	//delete birdParams;
	delete endSeq;
}

int FightMode::GetNumStoredBytes()
{
	return sizeof(MyData);// +testGator->GetNumStoredBytes() + testBird->GetNumStoredBytes();
}

void FightMode::StoreBytes(unsigned char *bytes)
{
	data.done = (int)done;
	data.endSeqState = endSeq->state;
	data.endSeqFrame = endSeq->frame;
	int dataSize = sizeof(MyData);
	memcpy(bytes, &data, dataSize);
	bytes += dataSize;

	/*testGator->StoreBytes(bytes);
	bytes += testGator->GetNumStoredBytes();
	testBird->StoreBytes(bytes);*/
}

void FightMode::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	done = (bool)data.done;
	endSeq->state = data.endSeqState;
	endSeq->frame = data.endSeqFrame;

	bytes += sizeof(MyData);
	/*testGator->SetFromBytes(bytes);
	bytes += testGator->GetNumStoredBytes();
	testBird->SetFromBytes(bytes);*/
}

void FightMode::Setup()
{
	maxHealth = 100;
	meterSection = 10;
	numMeterSections = 6;
}

void FightMode::StartGame()
{
	//testGator->Reset();
	//testBird->Reset();

	for (int i = 0; i < 4; ++i)
	{
		data.health[i] = maxHealth;
		data.meter[i] = 0;
	}
}

HUD *FightMode::CreateHUD()
{
	return new FightHUD;
}

bool FightMode::CheckVictoryConditions()
{
	if (done)
	{
		return false;
	}

	//fight should only be 2 players, like a fighitng game. can use deathmatch for more than 2

	bool playerDead[4];
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		playerDead[i] = false;

		p = sess->GetPlayer(i);

		if (p != NULL)
		{
			if (p->touchedGrass[Grass::HIT] || data.health[i] == 0 )
			{
				playerDead[i] = true;
			}
		}
	}

	if (playerDead[0])
	{
		sess->SetMatchPlacings(1, 0);
		return true;
	}
	else if (playerDead[1])
	{
		sess->SetMatchPlacings(0, 1);
		return true;
	}

	return false;
}

void FightMode::EndGame()
{
	cout << "game over" << endl;
	endSeq->Reset();
	sess->SetActiveSequence(endSeq);
	sess->ActiveSequenceUpdate();
	done = true;
	//sess->RestartGame();
}
