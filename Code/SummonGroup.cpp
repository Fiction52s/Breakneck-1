#include "SummonGroup.h"
#include "Enemy.h"
#include "Session.h"

SummonGroup::SummonGroup( Enemy *summoner, 
	int maxEnemies, ActorParams *p_enemyParams,
	int p_startMaxActive, int p_startSummonAtOnce)
{
	sess = Session::GetSession();

	numTotalEnemies = maxEnemies;
	enemyParams = p_enemyParams;
	enemies = new Enemy*[numTotalEnemies];
	startMaxActive = p_startMaxActive;
	startSummonAtOnce = p_startSummonAtOnce;

	for (int i = 0; i < numTotalEnemies; ++i)
	{
		enemies[i] = enemyParams->GenerateEnemy();
		enemies[i]->SetSummoner(summoner);
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
		enemies[i]->Reset();
	}
}

bool SummonGroup::CanSummon()
{
	numActiveEnemies < currMaxActiveEnemies;
}

void SummonGroup::Summon( PositionInfo &posInfo )
{
	int currSummoned = 0;
	for (int i = 0; i < numTotalEnemies; ++i)
	{
		if (!enemies[i]->active)
		{
			enemies[i]->spawned = false;
			enemies[i]->startPosInfo.SetWithoutChangingOffset(posInfo);

			sess->AddEnemy(enemies[i]);
			++numActiveEnemies;
			++currSummoned;

			if ( !CanSummon() )
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
