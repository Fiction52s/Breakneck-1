#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "GameSession.h"
#include "NetplayManager.h"
#include "ggponet.h"

using namespace std;
using namespace sf;


ParallelMode::ParallelMode()
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		parallelGames[i] = NULL;
	}
	ownsParallelSessions = true;
}

ParallelMode::~ParallelMode()
{
	if (ownsParallelSessions)
	{
		for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
		{
			if (parallelGames[i] != NULL)
			{
				delete parallelGames[i];
				parallelGames[i] = NULL;
			}
		}
	}
}

void ParallelMode::CreateParallelSessions()
{
	if (sess->IsSessTypeGame())
	{
		GameSession *game = GameSession::GetSession();
		if (game != NULL)
		{
			if (!game->IsParallelSession())
			{
				int numSessions = 0;
				if (game->gameModeType == MatchParams::GAME_MODE_PARALLEL_RACE)
				{
					numSessions = game->matchParams.numPlayers - 1;
				}
				else if (game->gameModeType == MatchParams::GAME_MODE_PARALLEL_PRACTICE)
				{
					numSessions = NetplayManager::MAX_PRACTICE_PLAYERS;
				}

				for (int i = 0; i < numSessions; ++i)
				{
					parallelGames[i] = game->CreateParallelSession(i);
				}
			}
		}
	}
}

void ParallelMode::UpdateParallelPlayerInputs()
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->UpdatePlayerInput(i + 1);
		}
	}
}

void ParallelMode::DrawParallelWires(sf::RenderTarget *target)
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
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
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
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

void ParallelMode::DrawParallelPlayerShields(sf::RenderTarget *target)
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			Actor *p = NULL;
			for (int j = 0; j < 4; ++j)
			{
				p = parallelGames[i]->GetPlayer(j);
				if (p != NULL)
				{
					p->DrawShield(target);
				}
			}
		}
	}
}

void ParallelMode::DrawParallelPlayerHomingBalls(sf::RenderTarget *target)
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			Actor *p = NULL;
			for (int j = 0; j < 4; ++j)
			{
				p = parallelGames[i]->GetPlayer(j);
				if (p != NULL)
				{
					p->DrawHomingBall(target);
				}
			}
		}
	}
}

void ParallelMode::DrawParallelNameTags(sf::RenderTarget *target)
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->DrawNameTags(target);
		}
	}
}

void ParallelMode::DrawParallelPlayersToMap(sf::RenderTarget *target, bool drawKin, bool drawNameTags )
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->DrawPlayersToMap( target, drawKin, drawNameTags);
		}
	}
}

void ParallelMode::DrawPracticeGames(sf::RenderTarget *target)
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->DrawPracticeGame(target);
		}
	}
}

void ParallelMode::UpdateParallelNameTagsPixelPos(sf::RenderTarget *target)
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->UpdateNameTagsPixelPos(target);
		}
	}
}

void ParallelMode::SimulateParallelGGPOGameFrames()
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->SimulateGGPOGameFrame();
		}
	}
}

void ParallelMode::ResetParallelTotalFramesIncludingRespawns()
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->totalGameFramesIncludingRespawns = 0;
		}
	}
}

void ParallelMode::SetParallelGGPOSessions(GGPOSession *p_ggpo)
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
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
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
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
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
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

void ParallelMode::RunParallelGGPOModeUpdates()
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->RunGGPOModeUpdate();
		}
	}
}

void ParallelMode::RunParallelMainLoopsOnce()
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->RunMainLoopOnce();
		}
	}
}

void ParallelMode::RestartParallelSessions()
{
	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
	{
		if (parallelGames[i] != NULL)
		{
			parallelGames[i]->RestartLevel();
		}
	}
}