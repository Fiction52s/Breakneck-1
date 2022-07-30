#include "MatchParams.h"

MatchParams::MatchParams()
{
	saveFile = NULL;
	netplayManager = NULL;
	numPlayers = 1;
	gameModeType = MatchParams::GAME_MODE_BASIC;
	randSeed = 0;
}

//void MatchParams::SetToNetplay(LobbyManager *lb, ConnectionManager *cm)
//{
//	lobbyManager = lb;
//	connectionManager = cm;
//}