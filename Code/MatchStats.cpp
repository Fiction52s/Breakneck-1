#include "MatchStats.h"
#include "Actor.h"

MatchStats::MatchStats()
{
	for (int i = 0; i < 4; ++i)
	{
		playerStats[i] = NULL;
	}
	netplay = false;
}
MatchStats::~MatchStats()
{
	for (int i = 0; i < 4; ++i)
	{
		if (playerStats[i] != NULL)
		{
			delete playerStats[i];
		}
	}
}

PlayerStats::PlayerStats(Actor * p)
{
	name = "test";
	skinIndex = 0;
	placing = -1;
	kills = 0;
}