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

ParallelPracticeMode::ParallelPracticeMode()
{
}

ParallelPracticeMode::~ParallelPracticeMode()
{

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
