#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"
#include "Enemy_Bird.h"
#include "FightEndSequence.h"
#include "GameSession.h"
#include "NetplayManager.h"

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

	if (testGame != NULL)
	{
		
		
		int winningIndex = -1;
		//int losingIndex = -1;
		if (sess->GetPlayer(0)->hitGoal)
		{
			winningIndex = sess->netplayManager->playerIndex;
			//sess->SetMatchPlacings(0, 1);
		}
		else if (testGame->GetPlayer(0)->hitGoal)
		{
			if (sess->netplayManager->playerIndex == 0)
			{
				winningIndex = 1;
			}
			else
			{
				winningIndex = 0;
			}
			
			//sess->SetMatchPlacings(0, 1);
		}

		if (winningIndex >= 0)
		{
			cout << "game ending. I am index: " << sess->netplayManager->playerIndex << endl;
			//cout << "wining index: " << winningIndex << ", losingIndex: " << losingIndex << endl;
			cout << "the winning player had an index of: " << sess->netplayManager->netplayPlayers[winningIndex].index << endl;
			cout << "current game frame: " << sess->totalGameFrames << endl;
			//cout << "the other player had an index of: " << sess->netplayManager->netplayPlayers[losingIndex].index << endl;
			return true;
		}
		
	}

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
