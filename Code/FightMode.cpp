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
	int dataSize = sizeof(MyData);
	memcpy(bytes, &data, dataSize);
	bytes += dataSize;
}

void FightMode::SetFromBuffer(unsigned char *buf)
{
	memcpy(&data, buf, sizeof(MyData));
}


void FightMode::Setup()
{
	maxHealth = 100;
	meterSection = 10;
	numMeterSections = 6;
}

void FightMode::StartGame()
{
	data.p0Health = maxHealth;
	data.p1Health = maxHealth;
	data.p0Meter = 0;
	data.p1Meter = 0;
}

HUD *FightMode::CreateHUD()
{
	return new FightHUD;
}

bool FightMode::CheckVictoryConditions()
{
	if (sess->GetPlayer(0)->touchedGrass[Grass::KILL]
		|| sess->GetPlayer(1)->touchedGrass[Grass::KILL]
		|| data.p0Health == 0 || data.p1Health == 0 )
	{
		return true;
	}
	return false;
}

void FightMode::EndGame()
{
	sess->RestartGame();
}
