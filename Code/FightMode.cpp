#include "Session.h"
#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"

using namespace std;
using namespace sf;

int FightMode::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void FightMode::StoreBytes(unsigned char *bytes)
{

}

void FightMode::SetFromBuffer(unsigned char *buf)
{

}


void FightMode::Setup()
{
}
void FightMode::StartGame()
{
}

HUD *FightMode::CreateHUD()
{
	return new FightHUD;
}

bool FightMode::CheckVictoryConditions()
{
	if (sess->GetPlayer(0)->touchedGrass[Grass::KILL]
		|| sess->GetPlayer(1)->touchedGrass[Grass::KILL])
	{
		return true;
	}
	return false;
}

void FightMode::EndGame()
{
	sess->RestartGame();
}
