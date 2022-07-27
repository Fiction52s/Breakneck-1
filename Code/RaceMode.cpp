#include "Session.h"
#include "GameMode.h"
#include "Actor.h"
#include "HUD.h"
#include "Enemy_Gator.h"
#include "Enemy_Bird.h"

using namespace std;
using namespace sf;

RaceMode::RaceMode()
{
}

RaceMode::~RaceMode()
{
}

int RaceMode::GetNumStoredBytes()
{
	return sizeof(MyData);
}

void RaceMode::StoreBytes(unsigned char *bytes)
{
	data.done = (int)done;
	int dataSize = sizeof(MyData);
	memcpy(bytes, &data, dataSize);
	bytes += dataSize;
}

void RaceMode::SetFromBytes(unsigned char *bytes)
{
	memcpy(&data, bytes, sizeof(MyData));

	done = (bool)data.done;

	bytes += sizeof(MyData);
}

void RaceMode::Setup()
{
}

void RaceMode::StartGame()
{
}

HUD *RaceMode::CreateHUD()
{
	return NULL;
	//return new FightHUD;
}

bool RaceMode::CheckVictoryConditions()
{
	if (done)
	{
		return false;
	}



	/*bool p0TouchedKillGrass = sess->GetPlayer(0)->touchedGrass[Grass::HIT];
	bool p1TouchedKillGrass = sess->GetPlayer(1)->touchedGrass[Grass::HIT];
	if (p0TouchedKillGrass || p1TouchedKillGrass
		|| data.p0Health == 0 || data.p1Health == 0)
	{
		return true;
	}
	return false;*/

	return false;
}

void RaceMode::EndGame()
{
	sess->RestartGame();
}
