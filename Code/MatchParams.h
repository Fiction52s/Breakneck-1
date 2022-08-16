#ifndef __MATCH_PARAMS_H__
#define __MATCH_PARAMS_H__

#include <boost/filesystem.hpp>
#include "steam\steam_api.h"

struct NetplayManager;
struct SaveFile;

struct MatchParams
{
	enum GameModeType
	{
		GAME_MODE_BASIC,
		GAME_MODE_REACHENEMYBASE,
		GAME_MODE_FIGHT,
		GAME_MODE_RACE,
		GAME_MODE_PARALLEL_RACE,
		GAME_MODE_Count,
	};

	boost::filesystem::path mapPath;
	SaveFile *saveFile;
	NetplayManager *netplayManager;
	//void SetToNetplay(NetplayManager *netplayManager);//LobbyManager *lb, ConnectionManager *cm);
	int numPlayers;
	int gameModeType;
	int randSeed;

	MatchParams();
	static std::string GetGameModeName(int gm);
	static bool IsMultiplayerMode(int gm);
	static std::vector<int> GetNumPlayerOptions(int gm);
};

#endif