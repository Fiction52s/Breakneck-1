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
	endSeq->SetIDAndAddToAllSequencesVec();
	endSeq->Init();

	std::vector<string> onlinePauseOptions = { "Resume", "Quit" };
	onlinePauseMenu = new BasicTextMenu(onlinePauseOptions);
}

FightMode::~FightMode()
{
	//delete gatorParams;
	//delete birdParams;
	delete endSeq;
}

void FightMode::KillPlayer(int index)
{
	int numAlive = 0;
	Actor *p = NULL;
	
	for (int i = 0; i < 4; ++i)
	{
		p = sess->GetPlayer(i);
		if (p != NULL && !p->dead)
		{
			++numAlive;
		}
	}

	int totalPlayers = sess->matchParams.numPlayers;

	int deathOrderNum = totalPlayers - numAlive;

	data.deathOrder[index] = deathOrderNum;
	
}

int FightMode::GetNumStoredBytes()
{
	return sizeof(MyData);// +testGator->GetNumStoredBytes() + testBird->GetNumStoredBytes();
}

void FightMode::StoreBytes(unsigned char *bytes)
{
	data.done = (int)done;
	//data.endSeqState = endSeq->state;
	//data.endSeqFrame = endSeq->frame;
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
	//endSeq->state = data.endSeqState;
	//endSeq->frame = data.endSeqFrame;

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
	done = false;

	for (int i = 0; i < 4; ++i)
	{
		data.health[i] = maxHealth;
		data.meter[i] = 0;
		data.killCounter[i] = 0;
		data.deathOrder[i] = -1;
	}
}

HUD *FightMode::CreateHUD()
{
	return new FightHUD( sess );
}

bool FightMode::CheckVictoryConditions()
{
	if (done)
	{
		return false;
	}

	//fight should only be 2 players, like a fighitng game. can use deathmatch for more than 2

	int numAlive = 0;
	Actor *p = NULL;
	for (int i = 0; i < 4; ++i)
	{
		p = sess->GetPlayer(i);
		if (p != NULL && !p->dead)
		{
			++numAlive;
		}
	}

	bool playerDead[4];
	
	if (numAlive == 1)
	{
		int placings[4];

		int victoryIndex = -1;
		for (int i = 0; i < 4; ++i)
		{
			p = sess->GetPlayer(i);
			if (p != NULL && !p->dead)
			{
				victoryIndex = i;
				break;
			}
		}

		placings[victoryIndex] = 0;
		for (int i = 0; i < 4; ++i)
		{
			if (i == victoryIndex)
				continue;

			p = sess->GetPlayer(i);
			if (p != NULL )
			{
				placings[i] = sess->matchParams.numPlayers - (data.deathOrder[i] + 1);
			}
			else
			{
				placings[i] = -1;
			}
		}

		
		sess->SetMatchPlacings(placings[0], placings[1], placings[2], placings[3]);
		return true;
	}
	//for (int i = 0; i < 4; ++i)
	//{
	//	playerDead[i] = false;

	//	p = sess->GetPlayer(i);

	//	if (p != NULL)
	//	{
	//		if (p->touchedGrass[Grass::HIT] || data.health[i] == 0 )
	//		{
	//			if (numAlive > 2)
	//			{
	//				//die? maybe have the player do this themselves??
	//			}
	//			else
	//			{
	//				playerDead[i] = true;
	//			}
	//			
	//		}
	//	}
	//}

	//if (playerDead[0])
	//{
	//	sess->SetMatchPlacings(1, 0);
	//	return true;
	//}
	//else if (playerDead[1])
	//{
	//	sess->SetMatchPlacings(0, 1);
	//	return true;
	//}

	return false;
}

void FightMode::EndGame()
{
	cout << "game over" << endl;
	endSeq->Reset();
	sess->SetActiveSequence(endSeq);
	sess->ActiveSequencesUpdate();
	done = true;
	//sess->RestartGame();
}
