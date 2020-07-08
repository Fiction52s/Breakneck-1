#include "Session.h"
#include "GameMode.h"

using namespace std;
using namespace sf;

GameMode::GameMode()
{
	sess = Session::GetSession();
	done = false;
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

ReachEnemyBaseMode::ReachEnemyBaseMode()
{

}

void ReachEnemyBaseMode::StartGame()
{

}

bool ReachEnemyBaseMode::CheckVictoryConditions()
{
	return false;
}

void ReachEnemyBaseMode::EndGame()
{

}