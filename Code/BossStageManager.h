#ifndef __BOSSSTAGEMANAGER_H__
#define __BOSSSTAGEMANAGER_H__

#include <vector>
#include "RandomPicker.h"

struct BossStageManager
{
	int totalHealth;
	std::vector<int> stageHits;
	std::vector<RandomPicker> decisionPickers;
	int currStage;
	int currStageHealth;
	int numStages;

	BossStageManager();
	void AddActiveOption(int stageIndex, int option, int reps = 1);
	void Setup(int numStages, int hitsPerStage);
	void Reset();
	void AddBossStage(int numHits);
	int AlwaysGetNextOption();
	int TryGetNextOption();
	int GetCurrStage();
	int GetTotalHealth();
	bool TakeHit();
};

#endif