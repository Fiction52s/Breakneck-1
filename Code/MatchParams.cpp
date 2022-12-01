#include "MatchParams.h"
#include <vector>

using namespace std;

MatchParams::MatchParams()
{
	Clear();
}

void MatchParams::Clear()
{
	saveFile = NULL;
	netplayManager = NULL;
	numPlayers = 1;
	gameModeType = MatchParams::GAME_MODE_BASIC;
	randSeed = 0;
	controllerStateVec.clear();
	controllerStateVec.resize(4);
	playerSkins.clear();
	playerSkins.resize(4);
}


bool MatchParams::HasControllerStates()
{
	for (int i = 0; i < 4; ++i)
	{
		if (controllerStateVec[i] != NULL)
		{
			return true;
		}
	}
	
	return false;
}

std::string MatchParams::GetGameModeName(int gm)
{
	switch (gm)
	{
	case GAME_MODE_BASIC:
		return "Basic";
		break;
	case GAME_MODE_REACHENEMYBASE:
		return "Reach Enemy Base";
		break;
	case GAME_MODE_FIGHT:
		return "Fight";
		break;
	case GAME_MODE_RACE:
		return "Race";
		break;
	case GAME_MODE_PARALLEL_RACE:
		return "Speedrun Race/Parallel Race";
		break;
	case GAME_MODE_EXPLORE:
		return "Explore";
		break;
	}

	return "game mode string failure";
}

bool MatchParams::IsMultiplayerMode(int gm)
{
	if (gm == GAME_MODE_BASIC)
	{
		return false;
	}

	return true;
}

std::vector<int> MatchParams::GetNumPlayerOptions(int gm, int numSpawns)
{
	vector<int> options;
	options.reserve(4); //max_players
	switch (gm)
	{
	case GAME_MODE_BASIC:
		options.push_back(1);
		break;
	case GAME_MODE_REACHENEMYBASE:
		break;
	case GAME_MODE_FIGHT:
		options.push_back(2);
		if (numSpawns >= 3 )
			options.push_back(3);
		if( numSpawns >= 4)
			options.push_back(4);
		break;
	case GAME_MODE_RACE:
		options.push_back(2);
		if (numSpawns >= 3)
		options.push_back(3);
		if (numSpawns >= 4)
		options.push_back(4);
		break;
	case GAME_MODE_PARALLEL_RACE:
		options.push_back(2);
		options.push_back(3);
		options.push_back(4);
		break;
	case GAME_MODE_EXPLORE:
		options.push_back(1);
		if( numSpawns >= 2 )
			options.push_back(2);
		if( numSpawns >= 3 )
			options.push_back(3);
		if (numSpawns >= 4)
			options.push_back(4);
		break;
	}

	return options;
}

//void MatchParams::SetToNetplay(LobbyManager *lb, ConnectionManager *cm)
//{
//	lobbyManager = lb;
//	connectionManager = cm;
//}