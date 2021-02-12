#include "BossStageManager.h"
#include <assert.h>

BossStageManager::BossStageManager()
{
	totalHealth = 0;
	currStageHealth = 0;
	numStages = 0;
}

void BossStageManager::AddBossStage(int numHits)
{
	stageHits.push_back(numHits);
	++numStages;
	totalHealth += numHits;
}

void BossStageManager::Reset()
{
	currStageHealth = stageHits[0];
	currStage = 0;
}

int BossStageManager::GetCurrStage()
{
	return currStage;
}

//return true if remain in same stage, return false if next stage
bool BossStageManager::TakeHit()
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
		return false;
	}

	return true;
}

int BossStageManager::GetTotalHealth()
{
	return totalHealth;
}
