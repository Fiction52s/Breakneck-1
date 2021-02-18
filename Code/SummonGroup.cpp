#include "SummonGroup.h"
#include "Enemy.h"
#include "Session.h"

SummonGroup::SummonGroup( Summoner *p_summoner, 
	ActorParams *p_enemyParams,
	int maxEnemies, int p_startMaxActive, int p_startSummonAtOnce,
	bool p_instantSummon)
{
	sess = Session::GetSession();

	numTotalEnemies = maxEnemies;
	enemyParams = p_enemyParams;
	enemies = new Enemy*[numTotalEnemies];
	startMaxActive = p_startMaxActive;
	startSummonAtOnce = p_startSummonAtOnce;
	summoner = p_summoner;
	instantSummon = p_instantSummon;

	for (int i = 0; i < numTotalEnemies; ++i)
	{
		enemies[i] = enemyParams->GenerateEnemy();
		enemies[i]->SetSummonGroup(this);
	}
}

SummonGroup::~SummonGroup()
{
	for (int i = 0; i < numTotalEnemies; ++i)
	{
		delete enemies[i];
	}
	delete[] enemies;

	delete enemyParams;
}

void SummonGroup::Reset()
{
	currMaxActiveEnemies = startMaxActive;
	numEnemiesToSummonAtOnce = startSummonAtOnce;
	numActiveEnemies = 0;

	for (int i = 0; i < numTotalEnemies; ++i)
	{
		if (enemies[i]->active)
		{
			sess->RemoveEnemy(enemies[i]);
		}
		enemies[i]->Reset();
	}
}

bool SummonGroup::CanSummon()
{
	return numActiveEnemies < currMaxActiveEnemies;
}

void SummonGroup::Summon()
{
	int currSummoned = 0;
	for (int i = 0; i < numTotalEnemies; ++i)
	{
		if (!enemies[i]->active)
		{
			enemies[i]->spawned = false;

			summoner->InitEnemyForSummon(this, enemies[i]);
			//enemies[i]->startPosInfo.SetWithoutChangingOffset(posInfo);

			sess->AddEnemy(enemies[i]);
			++numActiveEnemies;
			++currSummoned;

			if (!CanSummon())
			{
				break;
			}
			else if (currSummoned == numEnemiesToSummonAtOnce)
			{
				break;
			}
		}
	}
}

//only need this if the enemy shoots bullets
int SummonGroup::SetLaunchersStartIndex(int ind)
{
	for (int i = 0; i < numTotalEnemies; ++i)
	{
		ind = enemies[i]->SetLaunchersStartIndex(ind);
	}
	return ind;
}

void SummonGroup::HandleSummonedEnemyRemoval(Enemy *e)
{
	numActiveEnemies--;

	assert(numActiveEnemies >= 0);

	summoner->HandleSummonedChildRemoval(e);
}