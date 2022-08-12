#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"
#include "Enemy_Bird.h"
#include "FightEndSequence.h"
#include "GameSession.h"

using namespace std;
using namespace sf;

ParallelRaceMode::ParallelRaceMode()
{
	testGame = NULL;

	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		if (game != NULL)
		{
			if (!game->isParallelSession)
			{
				testGame = game->CreateParallelSession();
			}
		}
	}
	endSeq = new FightEndSequence;
	endSeq->Init();
}

ParallelRaceMode::~ParallelRaceMode()
{
	delete endSeq;

	if (testGame != NULL)
	{
		delete testGame;
	}
}

int ParallelRaceMode::GetNumStoredBytes()
{
	int total = sizeof(MyData);
	if (testGame != NULL)
	{
		total += testGame->GetNumStoredBytes();
	}
	return total;
}

void ParallelRaceMode::StoreBytes(unsigned char *bytes)
{
	data.done = (int)done;
	data.endSeqState = endSeq->state;
	data.endSeqFrame = endSeq->frame;
	int dataSize = sizeof(MyData);
	memcpy(bytes, &data, dataSize);
	bytes += dataSize;

	if (testGame != NULL)
	{
		testGame->StoreBytes(bytes);
		bytes += testGame->GetNumStoredBytes();
	}
}

void ParallelRaceMode::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	done = (bool)data.done;
	endSeq->state = data.endSeqState;
	endSeq->frame = data.endSeqFrame;

	bytes += sizeof(MyData);

	if (testGame != NULL)
	{
		testGame->SetFromBytes(bytes);
		bytes += testGame->GetNumStoredBytes();
	}
}

void ParallelRaceMode::Setup()
{
}

void ParallelRaceMode::StartGame()
{
}

HUD *ParallelRaceMode::CreateHUD()
{
	return NULL;
	//return new FightHUD;
}

bool ParallelRaceMode::CheckVictoryConditions()
{
	if (done)
	{
		return false;
	}

	/*if (sess->GetPlayer(0)->hitGoal)
	{
		return true;
	}
	else if (sess->GetPlayer(1)->hitGoal)
	{
		return true;
	}*/


	/*bool p0TouchedKillGrass = sess->GetPlayer(0)->touchedGrass[Grass::HIT];
	bool p1TouchedKillGrass = sess->GetPlayer(1)->touchedGrass[Grass::HIT];
	if (p0TouchedKillGrass || p1TouchedKillGrass
	|| data.p0Health == 0 || data.p1Health == 0)
	{
	return true;
	}
	return false;*/

	return false;
}

void ParallelRaceMode::EndGame()
{
	cout << "game over" << endl;
	endSeq->Reset();
	sess->SetActiveSequence(endSeq);
	sess->ActiveSequenceUpdate();
	done = true;
	//sess->RestartGame();
}
