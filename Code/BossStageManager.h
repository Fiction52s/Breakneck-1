#ifndef __BOSSSTAGEMANAGER_H__
#define __BOSSSTAGEMANAGER_H__

#include <vector>

struct BossStageManager
{
	int totalHealth;
	std::vector<int> stageHits;
	int currStage;
	int currStageHealth;
	int numStages;

	BossStageManager();
	void Reset();
	void AddBossStage(int numHits);
	int GetCurrStage();
	int GetTotalHealth();
	bool TakeHit();
};

#endif