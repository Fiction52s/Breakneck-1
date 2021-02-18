#include "BossStageManager.h"
#include <assert.h>

BossStageManager::BossStageManager()
{
	totalHealth = 0;
	currStageHealth = 0;
	numStages = 0;
}

void BossStageManager::Setup(int p_numStages, int hitsPerStage)
{
	numStages = p_numStages;
	stageHits.resize(numStages);
	for (int i = 0; i < numStages; ++i)
	{
		stageHits[i] = hitsPerStage;
	}
	totalHealth = numStages * hitsPerStage;
	decisionPickers.resize(numStages);
}

void BossStageManager::AddActiveOption(int stageIndex, int option, int reps)
{
	decisionPickers[stageIndex].AddActiveOption(option, reps);
}

void BossStageManager::AddBossStage(int numHits)
{
	stageHits.push_back(numHits);
	decisionPickers.push_back(RandomPicker());
	++numStages;
	totalHealth += numHits;
}

void BossStageManager::Reset()
{
	stageChanged = false;
	currStageHealth = stageHits[0];
	currStage = 0;
	for (int i = 0; i < numStages; ++i)
	{
		decisionPickers[i].ShuffleActiveOptions();
	}
}

int BossStageManager::AlwaysGetNextOption()
{
	return decisionPickers[currStage].AlwaysGetNextOption();
}

int BossStageManager::TryGetNextOption()
{
	return decisionPickers[currStage].TryGetNextOption();
}

int BossStageManager::GetCurrStage()
{
	return currStage;
}

//return true if remain in same stage, return false if next stage
void BossStageManager::TakeHit()
{
	--currStageHealth;
	if (currStageHealth == 0)
	{
		++currStage;
		if (currStage == numStages)
		{
			//should never happen
			currStage = 0;
			assert(0);
		}

		currStageHealth = stageHits[currStage];
		stageChanged = true;
	}
}

int BossStageManager::GetTotalHealth()
{
	return totalHealth;
}
