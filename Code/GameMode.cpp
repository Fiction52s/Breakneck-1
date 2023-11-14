#include "Session.h"
#include "GameMode.h"
#include "Actor.h"
#include "Enemy_MultiplayerBase.h"
#include "HUD.h"
#include "MapHeader.h"

using namespace std;
using namespace sf;

GameMode::GameMode()
{
	sess = Session::GetSession();
	done = false;
	onlinePauseMenu = NULL;
}

GameMode::~GameMode()
{
	if (onlinePauseMenu != NULL)
		delete onlinePauseMenu;
}

BasicMode::BasicMode()
{

}

void BasicMode::StartGame()
{

}

bool BasicMode::CheckVictoryConditions()
{
	if (sess->goalDestroyed)
	{
		return true;
	}
	return false;
}

void BasicMode::EndGame()
{
	sess->EndLevel();
}

HUD *BasicMode::CreateHUD()
{
	return new AdventureHUD( sess );
}

ExploreMode::ExploreMode( int p_numPlayers )
{
	numPlayers = p_numPlayers;
}

void ExploreMode::StartGame()
{

}

bool ExploreMode::CheckVictoryConditions()
{
	return false;
}

void ExploreMode::EndGame()
{
	sess->EndLevel();
}

HUD *ExploreMode::CreateHUD()
{
	if( numPlayers == 1 )
	{
		return new AdventureHUD( sess );
	}
	else
	{
		return NULL;
	}
}

ReachEnemyBaseMode::ReachEnemyBaseMode()
{
	ap0 = new BasicAirEnemyParams(sess->types["multiplayerbase"], 1);
	ap1 = new BasicAirEnemyParams(sess->types["multiplayerbase"], 1);
	ap0->CreateMyEnemy();//new MultiplayerBase(ap0);
	ap1->CreateMyEnemy();//new MultiplayerBase(ap1);
	p0Base = (MultiplayerBase*)ap0->myEnemy;
	p1Base = (MultiplayerBase*)ap1->myEnemy;

	p0Base->playerIndex = 1;
	p1Base->playerIndex = 0;
}

ReachEnemyBaseMode::~ReachEnemyBaseMode()
{
	//delete p0Base;
	//delete p1Base;
	delete ap0;
	delete ap1;
}

void ReachEnemyBaseMode::Setup()
{
	auto eParamsList = sess->groups["--"]->actors;
	enemies.clear();
	enemies.reserve(enemies.size() + 2);

	enemies.push_back(p0Base);
	enemies.push_back(p1Base);

	totalProgressTargets = 0;

	for (auto it = eParamsList.begin(); it != eParamsList.end(); ++it)
	{
		if ((*it)->myEnemy != NULL)
		{
			enemies.push_back((*it)->myEnemy);

			if ((*it)->GetTypeName() == "multiplayerprogresstarget")
			{
				++totalProgressTargets;
			}
		}
	}

	int extraBytes = 0;
	for (auto it = enemies.begin(); it != enemies.end(); ++it)
	{
		extraBytes += (*it)->GetNumStoredBytes();
	}

	totalStoredBytes = sizeof(ReachEnemyBaseModeData) + extraBytes;
}

void ReachEnemyBaseMode::StartGame()
{
	V2d p0 = sess->GetPlayerPos(0);
	V2d p1 = sess->GetPlayerPos(1);
	if (p0.x < p1.x)
	{
		sess->GetPlayer(1)->facingRight = false;
	}
	else if( p0.x > p1.x )
	{
		sess->GetPlayer(0)->facingRight = false;
	}

	p0Base->startPosInfo.SetPosition(p0);
	p1Base->startPosInfo.SetPosition(p1);

	p0Base->Reset();
	p1Base->Reset();

	//sess->AddEnemy(p0Base);
	//sess->AddEnemy(p1Base);

	p0Score = 0;
	p1Score = 0;

	p0HitTargets = 0;
	p1HitTargets = 1;

	//if I don't do this I get a desync probably caused by spawned variable
	for (auto it = enemies.begin(); it != enemies.end(); ++it)
	{
		//(*it)->Reset();
		sess->AddEnemy((*it));
	}
}

bool ReachEnemyBaseMode::CheckVictoryConditions()
{
	if (p0Base->dead || p1Base->dead )
	{
		return true;
	}
	return false;
}

void ReachEnemyBaseMode::EndGame()
{
	if (p0Base->dead)
	{
		p1Score++;
	}
	else if (p1Base->dead)
	{
		p0Score++;
	}

	if (p0Score == 3 || p1Score == 3 )
	{
		sess->EndLevel();
	}
	else
	{
		int tempP0Score = p0Score;
		int tempP1Score = p1Score;
		sess->RestartGame();

		p0Score = tempP0Score;
		p1Score = tempP1Score;
	}
}

int ReachEnemyBaseMode::GetNumStoredBytes()
{
	return totalStoredBytes;
}

void ReachEnemyBaseMode::StoreBytes(unsigned char *bytes)
{
	int dataSize = sizeof(ReachEnemyBaseModeData);

	ReachEnemyBaseModeData rd;
	memset(&rd, 0, dataSize);
	rd.p0Score = p0Score;
	rd.p1Score = p1Score;
	rd.p0HitTargets = p0HitTargets;
	rd.p1HitTargets = p1HitTargets;

	memcpy(bytes, &rd, dataSize);
	bytes += dataSize;

	for (auto it = enemies.begin(); it != enemies.end(); ++it)
	{
		(*it)->StoreBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}
}

void ReachEnemyBaseMode::SetFromBytes(unsigned char *bytes)
{
	int dataSize = sizeof(ReachEnemyBaseModeData);
	ReachEnemyBaseModeData rd;
	memcpy(&rd, bytes, dataSize);

	p0Score = rd.p0Score;
	p1Score = rd.p1Score;

	p0HitTargets = rd.p0HitTargets;
	p1HitTargets = rd.p1HitTargets;

	bytes += dataSize;

	for (auto it = enemies.begin(); it != enemies.end(); ++it)
	{
		(*it)->SetFromBytes(bytes);
		bytes += (*it)->GetNumStoredBytes();
	}
}


