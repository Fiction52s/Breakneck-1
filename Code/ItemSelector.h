#ifndef __ITEM_SELECTOR_H__
#define __ITEM_SELECTOR_H__

struct SingleAxisSelector
{
	SingleAxisSelector(int numWaitFramesLevels,
		int*p_waitFrames,
		int numWaitModeThreshLevels,
		int *p_waitModeThresh,
		int totalItems,
		int startIndex,
		bool loop = true);
	~SingleAxisSelector();
	void Reset();
	int *waitFrames;// [3];
	int *waitModeThresh;//[2];
	int framesWaiting;
	//0 for none, 1 for inc, -1 for dec
	int UpdateIndex(bool dec, bool inc);
	int currWaitLevel;
	int flipCounterUp;
	int flipCounterDown;
	bool loop;
	int currIndex;
	int totalItems;
	int oldCurrIndex;
	int startIndex;
};

#endif