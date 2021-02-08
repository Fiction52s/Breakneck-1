#ifndef __RANDOMPICKER_H__
#define __RANDOMPICKER_H__

#include <vector>

struct RandomPicker
{
	RandomPicker();
	~RandomPicker();
	void ReserveNumOptions(int m);
	void AddActiveOption(int a, int reps = 1);
	void Reset();
	void ShuffleActiveOptions();
	int GetNextOption();
	int AlwaysGetNextOption();
	std::vector<int> options;
	//int numMaxOptions;

	//int numActiveOptions;

	int currActiveIndex;

};

#endif