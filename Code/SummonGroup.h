#ifndef __SUMMONGROUP_H__
#define __SUMMONGROUP_H__

#include "PositionInfo.h"

struct ActorParams;
struct Enemy;
struct Session;

struct SummonGroup
{
	SummonGroup( Enemy *summoner,
		int maxEnemies,
		ActorParams *enemyParams,
		int startMaxActive,
		int startSummonAtOnce);
	void Reset();
	~SummonGroup();
	void Summon( PositionInfo &posInfo );
	bool CanSummon();
	ActorParams *enemyParams;
	int numTotalEnemies;
	Enemy **enemies;
	int numActiveEnemies;
	int currMaxActiveEnemies;
	int numEnemiesToSummonAtOnce;
	int startMaxActive;
	int startSummonAtOnce;
	Session *sess;
};

#endif