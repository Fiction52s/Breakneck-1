#include "Session.h"
#include "GameMode.h"

using namespace std;
using namespace sf;

GameMode::GameMode()
{
	sess = Session::GetSession();
}


BasicMode::BasicMode()
{

}

void BasicMode::StartGame()
{

}

void BasicMode::CheckVictoryConditions()
{

}

void BasicMode::EndGame()
{

}

ReachEnemyBaseMode::ReachEnemyBaseMode()
{

}

void ReachEnemyBaseMode::StartGame()
{

}

void ReachEnemyBaseMode::CheckVictoryConditions()
{

}

void ReachEnemyBaseMode::EndGame()
{

}