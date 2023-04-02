#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"
#include "Enemy_Bird.h"
#include "FightEndSequence.h"
#include "GameSession.h"
#include "NetplayManager.h"
#include "PracticeInviteDisplay.h"

using namespace std;
using namespace sf;

ParallelPracticeMode::ParallelPracticeMode()
{
	practiceInviteDisplay = NULL;
	if (!sess->IsParallelSession())
	{
		practiceInviteDisplay = new PracticeInviteDisplay;
	}
}

ParallelPracticeMode::~ParallelPracticeMode()
{
	if(practiceInviteDisplay != NULL )
		delete practiceInviteDisplay;
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

void ParallelPracticeMode::ResetInviteDisplay()
{
	practiceInviteDisplay->Reset();
}

bool ParallelPracticeMode::UpdateInviteDisplay(const ControllerState & curr, const ControllerState &prev)
{
	return practiceInviteDisplay->Update(curr, prev);
}

void ParallelPracticeMode::DrawInviteDisplay(sf::RenderTarget *target)
{
	practiceInviteDisplay->Draw(target);
}

//bool ParallelPracticeMode::ClearUpdateFlags()
//{
//	for (int i = 0; i < MAX_PARALLEL_SESSIONS; ++i)
//	{
//		updatePracticeSessions[i] = false;
//	}
//}