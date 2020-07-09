#include "Session.h"
#include "GameMode.h"
#include "Actor.h"
#include "Enemy_MultiplayerBase.h"

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
	ap0 = new BasicAirEnemyParams(sess->types["multiplayerbase"], 1);
	ap1 = new BasicAirEnemyParams(sess->types["multiplayerbase"], 1);
	ap0->CreateMyEnemy();//new MultiplayerBase(ap0);
	ap1->CreateMyEnemy();//new MultiplayerBase(ap1);
	p0Base = (MultiplayerBase*)ap0->myEnemy;
	p1Base = (MultiplayerBase*)ap1->myEnemy;

	p0Base->actorIndex = 0;
	p1Base->actorIndex = 1;
}

ReachEnemyBaseMode::~ReachEnemyBaseMode()
{
	//delete p0Base;
	//delete p1Base;
	delete ap0;
	delete ap1;
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

	sess->AddEnemy(p0Base);
	sess->AddEnemy(p1Base);

	p0Score = 0;
	p1Score = 0;
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

	//sess->EndLevel();
}