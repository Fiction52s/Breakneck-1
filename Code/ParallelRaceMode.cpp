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
	for (int i = 0; i < 3; ++i)
	{
		parallelGames[i] = NULL;
	}

	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		if (game != NULL)
		{
			if (!game->IsParallelSession())
			{
				for (int i = 0; i < game->matchParams.numPlayers-1; ++i)
				{
					parallelGames[i] = game->CreateParallelSession(i);
				}
			}
		}
	}
	endSeq = new FightEndSequence;
	endSeq->Init();
}

ParallelRaceMode::~ParallelRaceMode()
{
	delete endSeq;

	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			delete parallelGames[i];
		}
	}
}

int ParallelRaceMode::GetNumStoredBytes()
{
	int total = sizeof(MyData);

	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			total += parallelGames[i]->GetNumStoredBytes();
		}
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

	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->StoreBytes(bytes);
			bytes += parallelGames[i]->GetNumStoredBytes();
		}
	}
}

void ParallelRaceMode::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	done = (bool)data.done;
	endSeq->state = data.endSeqState;
	endSeq->frame = data.endSeqFrame;

	bytes += sizeof(MyData);

	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->SetFromBytes(bytes);
			bytes += parallelGames[i]->GetNumStoredBytes();
		}
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

	if (!sess->IsParallelSession())
	{
		/*Actor *p = NULL;
		for (int i = 0; i < 4; ++i)
		{
			p = sess->GetPlayer(i);
			if (p != NULL)
			{
				if (p->hitGoal)
				{
					sess->SetMatchPlacings(i);
					return true;
				}
			}
		}*/


		
		int winningIndex = -1;
		//int losingIndex = -1;
		if (sess->GetPlayer(0)->hitGoal)
		{
			winningIndex = sess->netplayManager->playerIndex;
			//sess->SetMatchPlacings(0, 1);
		}

		if (winningIndex == -1)
		{
			for (int i = 0; i < 3; ++i)
			{
				if (parallelGames[i] != NULL)
				{
					if (parallelGames[i]->GetPlayer(0)->hitGoal)
					{
						winningIndex = i;
						if (i >= sess->netplayManager->playerIndex)
						{
							++winningIndex;
						}

						break;

					}
				}
			}
		}

		if (winningIndex >= 0)
		{
			cout << "game ending. I am index: " << sess->netplayManager->playerIndex << endl;
			//cout << "wining index: " << winningIndex << ", losingIndex: " << losingIndex << endl;
			cout << "the winning player had an index of: " << sess->netplayManager->netplayPlayers[winningIndex].index << endl;
			cout << "current game frame: " << sess->totalGameFrames << endl;

			int placings[4];
			for (int i = 0; i < 4; ++i)
			{
				placings[i] = 1;
			}
			placings[sess->netplayManager->netplayPlayers[winningIndex].index] = 0;


			cout << "placings: " << placings[0] << ", " << placings[1] << ", " << placings[2] << ", " << placings[3] << "\n";
			sess->SetMatchPlacings(placings[0], placings[1], placings[2], placings[3]);
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
