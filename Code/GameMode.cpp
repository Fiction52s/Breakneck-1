#include "Session.h"
#include "GameMode.h"
#include "Actor.h"

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
}

bool ReachEnemyBaseMode::CheckVictoryConditions()
{
	return false;
}

void ReachEnemyBaseMode::EndGame()
{

}