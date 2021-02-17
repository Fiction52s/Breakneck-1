#include "RandomPicker.h"
#include <assert.h>
#include <random>

RandomPicker::RandomPicker()
{
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

int RandomPicker::AlwaysGetNextOption()
{
	int r = TryGetNextOption();
	if (r == -1)
	{
		ShuffleActiveOptions();
		r = TryGetNextOption();
	}
	return r;
}