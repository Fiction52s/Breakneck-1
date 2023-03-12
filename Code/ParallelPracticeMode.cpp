#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"
#include "Enemy_Bird.h"
#include "FightEndSequence.h"
#include "GameSession.h"
#include "NetplayManager.h"
#include "ggponet.h"

using namespace std;
using namespace sf;


ParallelMode::ParallelMode()
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
				for (int i = 0; i < game->matchParams.numPlayers - 1; ++i)
				{
					parallelGames[i] = game->CreateParallelSession(i);
				}
			}
		}
	}
}

ParallelMode::~ParallelMode()
{
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			delete parallelGames[i];
			parallelGames[i] = NULL;
		}
	}
}

void ParallelMode::UpdateParallelPlayerInputs()
{
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->UpdatePlayerInput(i + 1);
		}
	}
}

void ParallelMode::DrawParallelWires(sf::RenderTarget *target)
{
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			Actor *p = NULL;
			for (int j = 0; j < 4; ++j)
			{
				p = parallelGames[i]->GetPlayer(j);
				if (p != NULL)
				{
					p->DrawWires(target);
				}
			}
		}
	}
}

void ParallelMode::DrawParallelPlayers(sf::RenderTarget *target)
{
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			Actor *p = NULL;
			for (int j = 0; j < 4; ++j)
			{
				p = parallelGames[i]->GetPlayer(j);
				if (p != NULL)
				{
					//cout << "draw enemy player from world: " << i << " drawing player " << j << endl;
					p->Draw(target);
				}
			}
		}
	}
}

void ParallelMode::SimulateParallelGGPOGameFrames()
{
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->SimulateGGPOGameFrame();
		}
	}
}

void ParallelMode::SetParallelGGPOSessions(GGPOSession *p_ggpo)
{
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->ggpo = p_ggpo;
		}
	}
}

void ParallelMode::RespawnParallelPlayers()
{
	Actor *p = NULL;
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			p = parallelGames[i]->GetPlayer(0);
			p->Respawn();
		}
	}
}

void ParallelMode::SetParalellGGPOInputs(COMPRESSED_INPUT_TYPE *ggpoCompressedInputs)
{
	for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			for (int pIndex = 0; pIndex < GGPO_MAX_PLAYERS; ++pIndex)
			{
				parallelGames[i]->ggpoCompressedInputs[pIndex] = ggpoCompressedInputs[pIndex];
			}
		}
	}
}

ParallelPracticeMode::ParallelPracticeMode()
{
	/*for (int i = 0; i < 3; ++i)
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
				for (int i = 0; i < game->matchParams.numPlayers - 1; ++i)
				{
					parallelGames[i] = game->CreateParallelSession(i);
				}
			}
		}
	}*/
}

ParallelPracticeMode::~ParallelPracticeMode()
{
	/*for (int i = 0; i < 3; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			delete parallelGames[i];
		}
	}*/
}

void ParallelPracticeMode::Setup()
{
}

void ParallelPracticeMode::StartGame()
{
}

HUD *ParallelPracticeMode::CreateHUD()
{
	return new AdventureHUD;
}

bool ParallelPracticeMode::CheckVictoryConditions()
{
	if (done)
	{
		return false;
	}

	if (!sess->IsParallelSession())
	{
		if (sess->goalDestroyed)
		{
			return true;
		}
	}

	return false;
}

void ParallelPracticeMode::EndGame()
{
	done = true;

	sess->EndLevel();
	cout << "game over" << endl;
}
