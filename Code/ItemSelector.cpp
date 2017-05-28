#include "ItemSelector.h"

SingleAxisSelector::SingleAxisSelector(int numWaitFramesLevels, int*p_waitFrames, int numWaitModeThreshLevels,
	int *p_waitModeThresh, int p_totalItems, int p_startIndex, bool p_loop)
	:startIndex( p_startIndex ), loop(p_loop), currIndex(p_startIndex), totalItems(p_totalItems)
{
	waitFrames = new int[numWaitFramesLevels];
	for (int i = 0; i < numWaitFramesLevels; ++i)
	{
		waitFrames[i] = p_waitFrames[i];
	}

	waitModeThresh = new int[numWaitModeThreshLevels];
	for (int i = 0; i < numWaitFramesLevels; ++i)
	{
		waitModeThresh[i] = p_waitModeThresh[i];
	}

	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;
	oldCurrIndex = startIndex;
}

SingleAxisSelector::~SingleAxisSelector()
{
	delete[] waitFrames;
	delete[] waitModeThresh;
}

int SingleAxisSelector::UpdateIndex(bool dec, bool inc)
{
	oldCurrIndex = currIndex;
	if (inc)
	{
		if (flipCounterDown == 0
			|| (flipCounterDown > 0 && framesWaiting == waitFrames[currWaitLevel])
			)
		{
			if (flipCounterDown == 0)
			{
				currWaitLevel = 0;
			}

			++flipCounterDown;

			if (flipCounterDown == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
			{
				currWaitLevel++;
			}

			flipCounterUp = 0;
			framesWaiting = 0;

			if (currIndex < totalItems - 1)
			{
				currIndex++;
			}
			else
			{
				currIndex = 0;
			}
			return 1;
		}
		else
		{
			++framesWaiting;
		}

	}
	else if (dec)
	{
		if (flipCounterUp == 0
			|| (flipCounterUp > 0 && framesWaiting == waitFrames[currWaitLevel])
			)
		{
			if (flipCounterUp == 0)
			{
				currWaitLevel = 0;
			}

			++flipCounterUp;

			if (flipCounterUp == waitModeThresh[currWaitLevel] && currWaitLevel < 2)
			{
				currWaitLevel++;
			}

			flipCounterDown = 0;
			framesWaiting = 0;
			if (currIndex > 0)
			{
				currIndex--;
			}
			else
			{
				currIndex = totalItems - 1;
			}
			return -1;
		}
		else
		{
			++framesWaiting;
		}

	}
	else
	{
		flipCounterUp = 0;
		flipCounterDown = 0;
		currWaitLevel = 0;
		framesWaiting = 0;
		return 0;
	}
}

void SingleAxisSelector::Reset()
{
	currWaitLevel = 0;
	flipCounterUp = 0;
	flipCounterDown = 0;
	framesWaiting = 0;
	oldCurrIndex = startIndex;
	currIndex = startIndex;
}
