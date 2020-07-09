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
}

bool ReachEnemyBaseMode::CheckVictoryConditions()
{
	return false;
}

void ReachEnemyBaseMode::EndGame()
{

}