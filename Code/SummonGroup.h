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
		bool p_instantSummon = false);
	void Reset();
	~SummonGroup();
	void Summon();
	bool CanSummon();
	void DrawMinimap(sf::RenderTarget *target);
	int SetLaunchersStartIndex(int ind);
	void HandleSummonedEnemyRemoval(Enemy *e);
	void SetEnemyIDAndAddToAllEnemiesVec();
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