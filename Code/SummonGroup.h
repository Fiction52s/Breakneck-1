#ifndef __SUMMONGROUP_H__
#define __SUMMONGROUP_H__

#include "PositionInfo.h"

struct ActorParams;
struct Enemy;
struct Session;
struct SummonGroup;

struct Summoner
{
	virtual void InitEnemyForSummon(SummonGroup *group,
		Enemy *e) {}
	virtual void HandleSummonedChildRemoval(Enemy *e) {}
};

struct SummonGroup
{
	SummonGroup(Summoner *summoner,
		ActorParams *enemyParams,
		int maxEnemies,
		int startMaxActive,
		int startSummonAtOnce,
		int p_instantSummon = false);
	void Reset();
	~SummonGroup();
	void Summon();
	bool CanSummon();
	int SetLaunchersStartIndex(int ind);
	void HandleSummonedEnemyRemoval(Enemy *e);
	ActorParams *enemyParams;
	int numTotalEnemies;
	Summoner *summoner;
	Enemy **enemies;
	int numActiveEnemies;
	int currMaxActiveEnemies;
	int numEnemiesToSummonAtOnce;
	int startMaxActive;
	int startSummonAtOnce;
	Session *sess;
	bool instantSummon;
};

#endif