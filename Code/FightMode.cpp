#include "Session.h"
#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"

using namespace std;
using namespace sf;

FightMode::FightMode()
{
	gatorParams = new BasicAirEnemyParams(sess->types["gator"], 1);
	gatorParams->CreateMyEnemy();
	testGator = (Gator*)gatorParams->myEnemy;
}

FightMode::~FightMode()
{
	delete gatorParams;
}

int FightMode::GetNumStoredBytes()
{
	return sizeof(MyData) + testGator->GetNumStoredBytes();
}

void FightMode::StoreBytes(unsigned char *bytes)
{
	int dataSize = sizeof(MyData);
	memcpy(bytes, &data, dataSize);
	bytes += dataSize;

	testGator->StoreBytes(bytes);
}

void FightMode::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	bytes += sizeof(MyData);
	testGator->SetFromBytes(bytes);
}

void FightMode::Setup()
{
	maxHealth = 100;
	meterSection = 10;
	numMeterSections = 6;
}

void FightMode::StartGame()
{
	testGator->Reset();

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
