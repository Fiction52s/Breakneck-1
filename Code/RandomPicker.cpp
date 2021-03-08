#include "RandomPicker.h"
#include <assert.h>
#include <random>

RandomPicker::RandomPicker()
{
	preventShuffleRepetition = false;
}

RandomPicker::~RandomPicker()
{
}

void RandomPicker::ReserveNumOptions(int m)
{
	options.reserve(m);
}

void RandomPicker::Reset()
{
	options.resize(0);
}

void RandomPicker::AddActiveOption(int a, int reps)
{
	assert(reps > 0);
	for (int i = 0; i < reps; ++i)
	{
		options.push_back(a);
	}
}

void RandomPicker::ShuffleActiveOptions()
{
	int val;
	int index;
	int numActiveOptions = options.size();
	for (int i = numActiveOptions - 1; i >= 1; --i)
	{
		index = rand() % (i + 1);
		val = options[index];
		options[index] = options[i];
		options[i] = val;
	}
	currActiveIndex = 0;
}

int RandomPicker::TryGetNextOption()
{
	if (currActiveIndex >= options.size())
	{
		return -1;
	}
	else
	{
		int val = options[currActiveIndex];
		++currActiveIndex;
		return val;
	}
}

void RandomPicker::SetPreventShuffleRepetition(bool s)
{
	preventShuffleRepetition = s;
}

int RandomPicker::AlwaysGetNextOption()
{
	int r = TryGetNextOption();
	if (r == -1)
	{
		int lastOption = options.back();
		ShuffleActiveOptions();

		if (preventShuffleRepetition)
		{
			//prevents nodes from repeating from a shuffle
			if (options[0] == lastOption && options.size() > 2 )
			{
				int newI = (rand() % (options.size() - 2)) + 1;
				int temp = options[newI];
				options[newI] = options[0];
				options[0] = temp;
			}
		}

		r = TryGetNextOption();
	}
	return r;
}