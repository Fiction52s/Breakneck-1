#ifndef __MATCH_STATS_H__
#define __MATCH_STATS_H__

#include <string>

struct Actor;
struct PlayerStats
{
	PlayerStats(Actor * p);
	std::string name;
	int skinIndex;
	int placing;
	int kills;
};

struct MatchStats
{
	MatchStats();
	~MatchStats();
	bool netplay;
	PlayerStats *playerStats[4];
};

#endif